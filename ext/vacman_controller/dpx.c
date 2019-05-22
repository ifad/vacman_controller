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

  /* Open the DPX */
  if (result != 0) {
    vacman_library_error("AAL2DPXInit", result);
    return Qnil;
  }

  /* Get static vector for token activation code generation */
  aat_ascii sw_out_static_vector[4094+1];
  aat_int32 sw_out_static_vector_len = sizeof(sw_out_static_vector);
  result = AAL2DPXGetStaticVector(&dpx_handle,
                                  &g_KernelParms,
                                  sw_out_static_vector,
                                  &sw_out_static_vector_len);

  /* If no static vector is present, clear the buffer */
  if (result != 0) {
    memset(sw_out_static_vector, 0, sizeof(sw_out_static_vector));
  }

  /* Get the tokens */
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

    vacman_digipass_to_rbhash_sv(&dpdata, sw_out_static_vector, hash);

    rb_ary_push(list, hash);
  }

  AAL2DPXClose(&dpx_handle);

  return list;
}


/*
 * Generate token activation code
 */
VALUE vacman_dpx_generate_token_activation(VALUE module, VALUE token) {
  TDigipassBlob dpdata;

  aat_ascii static_vector[4094+1];
  vacman_rbhash_to_digipass_sv(token, &dpdata, static_vector, sizeof(static_vector));

  TDigipassBlob *dpdata_ary[8] = { &dpdata, 0, 0, 0, 0, 0, 0, 0 };

  aat_int32 actv_flags = ACTV_OFFLINE;
  aat_ascii serial_num[14+1];
  aat_ascii actv_code[4142+1];

  aat_int32 result = AAL2GenActivationCodeXErc(dpdata_ary,     /* DPData */
                                               1,              /* Appl_count */
                                               &g_KernelParms, /* CallParms */
                                               static_vector,  /* aStaticVectorIn DIGIPASS parameter setting */
                                               NULL,           /* aSharedData for encryption */
                                               NULL,           /* aAlea for encryption */
                                               &actv_flags,    /* ActivationFlags */
                                               serial_num,     /* aSerialNumberSuffix */
                                               actv_code,      /* aXFAD */
                                               NULL);          /* aXERC */

  if (result != 0) {
    vacman_library_error("AAL2GenActivationCodeXErc", result);
    return Qnil;
  }

  VALUE ret = rb_hash_new();
  rb_hash_aset(ret, rb_str_new2("serial"), rb_str_new2(serial_num));
  rb_hash_aset(ret, rb_str_new2("activation"), rb_str_new2(actv_code));

  return ret;
}
