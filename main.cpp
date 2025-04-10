#include "header.hpp"

#include <apt-pkg/cachefile.h>
#include <apt-pkg/error.h>
#include <apt-pkg/init.h>
#include <apt-pkg/pkgcache.h>
#include <apt-pkg/pkgsystem.h>

#include <curl/curl.h>
#include <iostream>

static size_t WriteCallback(void const *const contents, size_t size,
                            size_t nmemb, void const *const userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

static int DoPost(std::string url) {
  CURL *curl;

  CURLcode res;

  std::string readBuffer;

  curl = curl_easy_init();

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    std::cout << readBuffer << std::endl;
  }

  return 0;
}

int main(int argc, char *argv[]) {

  std::string tomlConfigPath;
  std::vector<std::string> args;

  for (int i = 0; i < argc; i++) {
    args.push_back(argv[i]);
  }

  if (argc < 3) {
    std::cerr << "No config file provided by --config. exiting...\n";
    exit(1);
  }

  if (args[1] == "--config") {
    tomlConfigPath = args[2];
  } else {
    std::cerr << "Unknown option: " << args[1];
  }

  pkgInitConfig(*_config);
  pkgInitSystem(*_config, _system);

  pkgCacheFile CacheFile;
  pkgCache *Cache = CacheFile.GetPkgCache();

  if (!Cache) {
    std::cerr << "Error: Could not acquire cache." << std::endl;
    return 1;
  }

  for (pkgCache::PkgIterator P = Cache->PkgBegin(); P.end() != true; ++P) {
    if (P->CurrentVer != 0) {
      pkgCache::VerIterator V = P.CurrentVer();
      std::cout << P.FullName() << "::" << V.VerStr() << "\n";
    }
  }

  return 0;
}
