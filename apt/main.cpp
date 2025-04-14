#include "header.hpp"

#include <apt-pkg/cachefile.h>
#include <apt-pkg/error.h>
#include <apt-pkg/init.h>
#include <apt-pkg/pkgcache.h>
#include <apt-pkg/pkgsystem.h>
#include <curl/curl.h>
#include <iostream>
#include <limits.h>
#include <nlohmann/json.hpp>
#include <unistd.h>
#include <unordered_map>

using json = nlohmann::json;

static size_t WriteCallback(void const *const contents, size_t size,
                            size_t nmemb, void const *const userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

static int DoPost(std::string url, json object) {
  curl_global_init(CURL_GLOBAL_ALL);
  CURL *curl = curl_easy_init();
  CURLcode res;

  std::string readBuffer;

  struct curl_slist *slist1 = NULL;

  std::string json_dump = object.dump();

  if (curl) {
    slist1 = curl_slist_append(
        slist1, "Content-Type: application/json; charset: utf-8");
    slist1 = curl_slist_append(slist1, "Accept: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist1);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)json_dump.size());
    curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, json_dump.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "idunno-via-libcurl");

    res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
    }

    std::cout << readBuffer << std::endl;

    curl_easy_cleanup(curl);
    curl_slist_free_all(slist1);
  }

  curl_global_cleanup();
  return 0;
}

static std::unordered_map<std::string, std::string> GetPackages() {
  std::unordered_map<std::string, std::string> result;

  pkgInitConfig(*_config);
  pkgInitSystem(*_config, _system);

  pkgCacheFile CacheFile;
  pkgCache *Cache = CacheFile.GetPkgCache();

  if (!Cache) {
    std::cerr << "Error: Could not acquire cache." << std::endl;
    return result;
  }

  for (pkgCache::PkgIterator P = Cache->PkgBegin(); P.end() != true; ++P) {
    if (P->CurrentVer != 0) {
      pkgCache::VerIterator V = P.CurrentVer();

      result[P.FullName()] = V.VerStr();
    }
  }

  return result;
}

static json GetJSON(std::unordered_map<std::string, std::string> data) {
  int res;
  /* json object(data); */
  json object;
  json pkgs(data);
  object["timestamp"] = std::time(nullptr);
  char hostname[HOST_NAME_MAX];

  res = gethostname(hostname, HOST_NAME_MAX - 1);
  if (res != 0) {
  }

  object["hostname"] = std::string(hostname);

  object["packages"] = json::array({});

  int counter = 0;

  for (auto pkg : data) {
    object["packages"][counter] = {{"pkg_name", pkg.first},
                                   {"pkg_version", pkg.second}};
    counter++;
  }

  std::cout << object << "\n";

  return object;
}

int main(int argc, char *argv[]) {

  std::string tomlConfigPath;

  auto packages = GetPackages();

  auto object = GetJSON(packages);

  DoPost("127.0.0.1:4242", object);

  return 0;
}
