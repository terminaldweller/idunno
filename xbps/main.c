
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xbps.h>

int list_pkgs_in_dict(struct xbps_handle *xhp UNUSED, xbps_object_t obj,
                      const char *key UNUSED, void *arg,
                      bool *loop_done UNUSED) {
  struct list_pkgver_cb *lpc = arg;
  const char *pkgver = NULL, *short_desc = NULL, *state_str = NULL;
  unsigned int len;
  pkg_state_t state;

  xbps_dictionary_get_cstring_nocopy(obj, "pkgver", &pkgver);
  xbps_dictionary_get_cstring_nocopy(obj, "short_desc", &short_desc);
  if (!pkgver || !short_desc)
    return EINVAL;

  xbps_pkg_state_dictionary(obj, &state);

  if (state == XBPS_PKG_STATE_INSTALLED)
    state_str = "ii";
  else if (state == XBPS_PKG_STATE_UNPACKED)
    state_str = "uu";
  else if (state == XBPS_PKG_STATE_HALF_REMOVED)
    state_str = "hr";
  else
    state_str = "??";

  if (lpc->linebuf == NULL) {
    printf("%s %-*s %s\n", state_str, lpc->pkgver_len, pkgver, short_desc);
    return 0;
  }

  len = snprintf(lpc->linebuf, lpc->maxcols, "%s %-*s %s", state_str,
                 lpc->pkgver_len, pkgver, short_desc);
  /* add ellipsis if the line was truncated */
  if (len >= lpc->maxcols && lpc->maxcols > 4) {
    for (unsigned int j = 0; j < 3; j++)
      lpc->linebuf[lpc->maxcols - j - 1] = '.';
    lpc->linebuf[lpc->maxcols] = '\0';
  }
  puts(lpc->linebuf);

  return 0;
}
unsigned int find_longest_pkgver(struct xbps_handle *xhp, xbps_object_t o) {
  struct fflongest ffl;

  ffl.d = o;
  ffl.len = 0;

  if (xbps_object_type(o) == XBPS_TYPE_DICTIONARY) {
    xbps_array_t array;

    array = xbps_dictionary_all_keys(o);
    (void)xbps_array_foreach_cb_multi(xhp, array, o, _find_longest_pkgver_cb,
                                      &ffl);
    xbps_object_release(array);
  } else {
    (void)xbps_pkgdb_foreach_cb_multi(xhp, _find_longest_pkgver_cb, &ffl);
  }

  return ffl.len;
}

static int list_pkgs_pkgdb(struct xbps_handle const *const xh) {
  struct list_pkgver_cb lpc;

  return xbps_pkgdb_foreach_cb(xhp, list_pkgs_in_dict, &lpc);
}

int main(int argc, char **argv) {

  struct xbps_handle *xh;

  xbps_init(xh);

  list_pkgs_pkgdb(xh);

  xbps_end(xh);

  return 0;
}
