#include "header.hpp"

#include <curl/curl.h>
#include <libdnf5/base/base.hpp>
#include <libdnf5/repo/repo.hpp>
#include <libdnf5/repo/repo_sack.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <nlohmann/json.hpp>

static size_t WriteCallback(void const *const contents, size_t size,
                            size_t nmemb, void const *const userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

static int DoPost(std::string url, nlohmann::json object) {
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
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist1);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)json_dump.size());
    curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, json_dump.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "idunno-via-libcurl");
#ifndef SKIP_PEER_VERIFICATION
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif
#ifndef SKIP_HOSTNAME_VERIFICATION
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
    curl_easy_setopt(curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);

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

  libdnf5::Base base;
  base.get_config().get_installroot_option().set("/");
  base.get_config().get_plugins_option().set(false);
  base.load_config();
  base.setup();

  auto repo_sack = base.get_repo_sack();

  repo_sack->load_repos(libdnf5::repo::Repo::Type::SYSTEM);
  libdnf5::rpm::PackageQuery pkgs(base);
  pkgs.filter_installed();

  for (const auto &pkg : pkgs) {
    result[pkg.get_name()] = pkg.get_version();
  }

  return result;
}

static nlohmann::json
GetJSON(std::unordered_map<std::string, std::string> data) {
  int res;
  nlohmann::json object;
  nlohmann::json pkgs(data);
  object["timestamp"] = std::time(nullptr);
  char hostname[HOST_NAME_MAX];

  res = gethostname(hostname, HOST_NAME_MAX - 1);
  if (res != 0) {
  }

  object["hostname"] = std::string(hostname);

  object["packages"] = nlohmann::json::array({});

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
  auto packages = GetPackages();
  auto object = GetJSON(packages);

  DoPost("https://127.0.0.1:4242/api/v1/intake", object);

  return 0;
}
