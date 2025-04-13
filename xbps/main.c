#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cjson/cJSON.h>

#include <xbps.h>

#define UNUSED __attribute__((__unused__))
#define MAX_HOSTNAME_LEN 255

int make_json_object(cJSON *monitor) {
  int res;
  cJSON *hostname_json = NULL;
  char *hostname = NULL;
  cJSON *packages = cJSON_CreateArray();
  cJSON *package = cJSON_CreateObject();
  cJSON *timestamp = NULL;

  if (!monitor) {
    fprintf(stderr, "Failed to create JSON object\n");
    return -1;
  }

  hostname = malloc(sizeof(char) * (MAX_HOSTNAME_LEN + 1));
  res = gethostname(hostname, MAX_HOSTNAME_LEN);
  if (res != 0) {
    fprintf(stderr, "Failed to get hostname\n");
    return -1;
  }
  if (!hostname) {
    fprintf(stderr, "Failed to get hostname\n");
    return -1;
  }
  hostname_json = cJSON_CreateString(hostname);
  if (!hostname_json) {
    fprintf(stderr, "Failed to create JSON string\n");
    return -1;
  }
  cJSON_AddItemToObject(monitor, "hostname", hostname_json);

  timestamp = cJSON_CreateNumber(time(NULL));
  if (!timestamp) {
    fprintf(stderr, "Failed to create JSON number\n");
    return -1;
  }
  cJSON_AddItemToObject(monitor, "timestamp", timestamp);

  packages = cJSON_CreateArray();
  if (!packages) {
    fprintf(stderr, "Failed to create JSON array\n");
    return -1;
  }
  cJSON_AddItemToObject(monitor, "packages", packages);

  return 0;
}

int do_post(char const *const url, char const *const json_string) {
  CURL *curl;
  CURLcode res;

  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();

  struct curl_slist *slist1 = NULL;

  if (curl) {
    slist1 = curl_slist_append(
        slist1, "Content-Type: application/json; charset: utf-8");
    slist1 = curl_slist_append(slist1, "Accept: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(json_string));
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist1);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    }
    curl_easy_cleanup(curl);
    curl_slist_free_all(slist1);
  }

  curl_global_cleanup();

  return 0;
}

int list_pkgs_in_dict(struct xbps_handle *xhp UNUSED, xbps_object_t obj,
                      const char *key UNUSED, void *arg UNUSED,
                      bool *loop_done UNUSED) {
  char *pkgver = NULL;

  xbps_dictionary_get_cstring(obj, "pkgver", &pkgver);

  char *name, *version;
  cJSON *package = NULL;
  cJSON *pkg_name = NULL;
  cJSON *pkg_version = NULL;
  cJSON *packages = NULL;
  cJSON *monitor = (cJSON *)arg;

  xbps_dictionary_get_cstring(obj, "pkgname", &name);

  char *index = strrchr(pkgver, '-');

  char *version_string = strdup(index + 1);

  package = cJSON_CreateObject();
  if (!package) {
    fprintf(stderr, "Failed to create JSON object\n");
    return -1;
  }
  packages = cJSON_GetObjectItemCaseSensitive(monitor, "packages");
  cJSON_AddItemToArray(packages, package);

  pkg_name = cJSON_CreateString(strdup(name));
  if (!pkg_name) {
    fprintf(stderr, "Failed to create JSON string\n");
    return -1;
  }
  cJSON_AddItemToObject(package, "pkg_name", pkg_name);

  pkg_version = cJSON_CreateString(strdup(version_string));
  if (!pkg_version) {
    fprintf(stderr, "Failed to create JSON string\n");
    return -1;
  }
  cJSON_AddItemToObject(package, "pkg_version", pkg_version);

  return 0;
}

static int list_pkgs_pkgdb(struct xbps_handle *xh, cJSON *monitor) {
  make_json_object(monitor);
  if (!monitor) {
    fprintf(stderr, "Failed to create JSON object\n");
    return -1;
  }

  return xbps_pkgdb_foreach_cb(xh, list_pkgs_in_dict, monitor);
}

int main(int argc, char **argv) {
  int rv;

  struct xbps_handle xh;
  memset(&xh, 0, sizeof(xh));

  if ((rv = xbps_init(&xh)) != 0) {
    fprintf(stderr, "xbps_init failed\n");
    return rv;
  }

  cJSON *monitor = cJSON_CreateObject();

  list_pkgs_pkgdb(&xh, monitor);

  char *json_string = cJSON_Print(monitor);
  if (!json_string) {
    fprintf(stderr, "Failed to print JSON string\n");
    return -1;
  }
  cJSON_Delete(monitor);
  printf("%s\n", json_string);

  do_post("127.0.0.1:4242", json_string);

  xbps_end(&xh);

  return 0;
}
