/*
 * Vacman Controller wrapper
 *
 * This Ruby Extension wraps the VASCO Vacman Controller
 * library and makes its API accessible to Ruby code.
 *
 * (C) 2013 https://github.com/mlankenau
 * (C) 2019 m.barnaba@ifad.org
 */
#include "vacman_controller.h"

static VALUE rbhash_get_key(VALUE token, const char *property, int type);

/*
 * Convert a Ruby Hash with the required keys to a TDigipassBlob structure.
 */
void vacman_rbhash_to_digipass(VALUE token, TDigipassBlob* dpdata) {
  if (!RB_TYPE_P(token, T_HASH)) {
    rb_raise(e_VacmanError, "invalid token object given, requires an hash");
    return;
  }

  VALUE blob     = rbhash_get_key(token, "blob",     T_STRING);
  VALUE serial   = rbhash_get_key(token, "serial",   T_STRING);
  VALUE app_name = rbhash_get_key(token, "app_name", T_STRING);
  VALUE flag1    = rbhash_get_key(token, "flags1",   T_FIXNUM);
  VALUE flag2    = rbhash_get_key(token, "flags2",   T_FIXNUM);

  memset(dpdata, 0, sizeof(*dpdata));

  strcpy(dpdata->Blob, rb_string_value_cstr(&blob));
  strncpy(dpdata->Serial, rb_string_value_cstr(&serial), sizeof(dpdata->Serial));
  strncpy(dpdata->AppName, rb_string_value_cstr(&app_name), sizeof(dpdata->AppName));
  dpdata->DPFlags[0] = rb_fix2int(flag1);
  dpdata->DPFlags[1] = rb_fix2int(flag2);
}

/*
 * Convert a TDigipassBlob structure into a Ruby Hash
 */
void vacman_digipass_to_rbhash(TDigipassBlob* dpdata, VALUE hash) {
  char buffer[256];

  memset(buffer, 0, sizeof(buffer));
  strncpy(buffer, dpdata->Serial, 10);
  rb_hash_aset(hash, rb_str_new2("serial"), rb_str_new2(buffer));

  memset(buffer, 0, sizeof(buffer));
  strncpy(buffer, dpdata->AppName, 12);
  rb_hash_aset(hash, rb_str_new2("app_name"), rb_str_new2(buffer));

  memset(buffer, 0, sizeof(buffer));
  strncpy(buffer, dpdata->Blob, 224);
  rb_hash_aset(hash, rb_str_new2("blob"), rb_str_new2(buffer));

  rb_hash_aset(hash, rb_str_new2("flags1"), rb_fix_new(dpdata->DPFlags[0]));
  rb_hash_aset(hash, rb_str_new2("flags2"), rb_fix_new(dpdata->DPFlags[1]));
}

/*
 * Gets the given property from the given token hash and raises an Error
 * if the following conditions occur:
 *
 * * The key is not found
 * * The key is not of the given type
 *
 * Otherwise, the value corresponding to the key is returned.
 *
 */
static VALUE rbhash_get_key(VALUE token, const char *property, int type) {
  VALUE ret = rb_hash_aref(token, rb_str_new2(property));

  if (ret == Qnil) {
    rb_raise(e_VacmanError, "invalid token object given: %s property is nil", property);
    return Qnil;
  }

  if (!RB_TYPE_P(ret, type)) {
    rb_raise(e_VacmanError, "invalid token object given: %s property is not of the correct type", property);
    return Qnil;
  }

  return ret;
}
