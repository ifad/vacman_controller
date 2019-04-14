#ifndef __vacman_controller_h__
#define __vacman_controller_h__

#if defined(__cplusplus)
extern "C" {
#if 0
} /* satisfy cc-mode */
#endif
#endif

#include <ruby.h>
#include <string.h>
#include <aal2sdk.h>

/* The Vacman default kernel parameters, set up upon extension initialisation. */
TKernelParms g_KernelParms;

/* Ruby exception type, defined as VacmanController::Error in Ruby land. */
VALUE e_VacmanError;

/* General methods (main.c) */
void vacman_library_error(const char* method, int vacman_error_code);
VALUE vacman_library_version(VALUE unused);

/* Kernel methods (kernel.c) */
VALUE vacman_kernel_get_property_names();
VALUE vacman_kernel_get_param(VALUE module, VALUE paramname);
VALUE vacman_kernel_set_param(VALUE module, VALUE paramname, VALUE rbval);
void vacman_kernel_init_params();

/* Token methods (token.c) */
VALUE vacman_token_get_property_names();
VALUE vacman_token_get_property(VALUE module, VALUE token, VALUE property);
VALUE vacman_token_set_property(VALUE module, VALUE token, VALUE property, VALUE rbval);
VALUE vacman_token_set_pin(VALUE module, VALUE token, VALUE pin);
VALUE vacman_token_verify_password(VALUE module, VALUE token, VALUE password);
VALUE vacman_token_generate_password(VALUE module, VALUE token);

/* Token interchange format between Ruby and libaal2 (serialize.c) */
void vacman_rbhash_to_digipass(VALUE token, TDigipassBlob* dpdata);
void vacman_digipass_to_rbhash(TDigipassBlob* dpdata, VALUE hash);

/* DPX methods (dpx.c) */
VALUE vacman_dpx_import(VALUE module, VALUE filename, VALUE key);

#if defined(__cplusplus)
#if 0
{ /* satisfy cc-mode */
#endif
}  /* extern "C" { */
#endif

#endif /* __vacman_controller_h__ */
