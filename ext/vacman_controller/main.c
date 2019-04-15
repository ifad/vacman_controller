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

/*
 * Extension entry point
 */
void Init_vacman_low_level(void) {
  VALUE controller = rb_define_module("VacmanController");
  VALUE lowlevel   = rb_define_module_under(controller, "LowLevel");

  e_VacmanError = rb_define_class_under(controller, "Error", rb_eStandardError);

  vacman_kernel_init_params();

  /* Global methods */
  rb_define_singleton_method(lowlevel, "library_version",       vacman_library_version, 0);
  rb_define_singleton_method(lowlevel, "import",                vacman_dpx_import, 2);

  /* Token methods */
  rb_define_singleton_method(lowlevel, "token_property_names",  vacman_token_get_property_names, 0);
  rb_define_singleton_method(lowlevel, "get_token_property",    vacman_token_get_property, 2);
  rb_define_singleton_method(lowlevel, "set_token_property",    vacman_token_set_property, 3);
  rb_define_singleton_method(lowlevel, "set_token_pin",         vacman_token_set_pin, 2);
  rb_define_singleton_method(lowlevel, "verify_password",       vacman_token_verify_password, 2);
  rb_define_singleton_method(lowlevel, "generate_password",     vacman_token_generate_password, 1);

  /* Kernel methods */
  rb_define_singleton_method(lowlevel, "kernel_property_names", vacman_kernel_get_property_names, 0);
  rb_define_singleton_method(lowlevel, "get_kernel_param",      vacman_kernel_get_param, 1);
  rb_define_singleton_method(lowlevel, "set_kernel_param",      vacman_kernel_set_param, 2);
}

/*
 * Raises an Error, decoding the Vacman Controller error code.
 */
void vacman_library_error(const char* method, int vacman_error_code) {
  aat_ascii vacman_error_message[100]; // Recommended value in documentation.

  AAL2GetErrorMsg(vacman_error_code, vacman_error_message);

  char error_message[256];
  snprintf(error_message, 255, "%s error %d: %s", method, vacman_error_code,
           vacman_error_message);

  VALUE exc = rb_exc_new2(e_VacmanError, error_message);
  rb_iv_set(exc, "@library_method", rb_str_new2(method));
  rb_iv_set(exc, "@error_code",     INT2FIX(vacman_error_code));
  rb_iv_set(exc, "@error_message",  rb_str_new2(vacman_error_message));

  rb_exc_raise(exc);
}


/*
 * Use AAL2GetLibraryVersion to obtain library version and return it as a Ruby Hash
 */
VALUE vacman_library_version(VALUE unused) {
  aat_ascii version[16];
  aat_int32 version_len = sizeof(version);

  aat_ascii bitness[4];
  aat_int32 bitness_len = sizeof(bitness);

  aat_ascii type[8];
  aat_int32 type_len = sizeof(type);

  aat_int32 result = AAL2GetLibraryVersion(version, &version_len, bitness,
      &bitness_len, type, &type_len);

  if (result != 0) {
    vacman_library_error("AAL2GetLibraryVersion", result);
    return Qnil;
  }

  VALUE hash = rb_hash_new();
  rb_hash_aset(hash, rb_str_new2("version"), rb_str_new2(version));
  rb_hash_aset(hash, rb_str_new2("bitness"), rb_str_new2(bitness));
  rb_hash_aset(hash, rb_str_new2("type"),    rb_str_new2(type));

  return hash;
}
