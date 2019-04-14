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
 * Imports a .DPX file containing token seeds and initialisation values.
 *
 * Pass the pre-shared key to validate it as the second argument. The
 * key is not validated by the AAL2 library, if you pass a different
 * key than the one that was used to create the DPX, you will get back
 * tokens that generate different OTPs.
 *
 */
VALUE vacman_dpx_import(VALUE module, VALUE filename, VALUE key) {
  TDPXHandle dpx_handle;
  aat_int16  appl_count;
  aat_ascii  appl_names[13*8];
  aat_int16  token_count;

  aat_int32 result = AAL2DPXInit(&dpx_handle,
                                 rb_string_value_cstr(&filename),
                                 rb_string_value_cstr(&key),
                                 &appl_count,
                                 appl_names,
                                 &token_count);

  if (result != 0) {
    vacman_library_error("AAL2DPXInit", result);
    return Qnil;
  }

  aat_ascii sw_out_serial_No[22+1];
  aat_ascii sw_out_type[5+1];
  aat_ascii sw_out_authmode[2+1];
  TDigipassBlob dpdata;

  VALUE list = rb_ary_new();

  while (1) {
    result = AAL2DPXGetToken(&dpx_handle,
        &g_KernelParms,
        appl_names,
        sw_out_serial_No,
        sw_out_type,
        sw_out_authmode,
        &dpdata);


    if (result < 0) {
      vacman_library_error("AAL2DPXGetToken", result);
      return Qnil;
    }

    if (result == 107) break;

    VALUE hash = rb_hash_new();

    vacman_digipass_to_rbhash(&dpdata, hash);

    rb_ary_push(list, hash);
  }

  AAL2DPXClose(&dpx_handle);

  return list;
}
