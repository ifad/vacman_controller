#include <ruby.h>
#include <string.h>
#include <aal2sdk.h>

static VALUE e_vacmanerror;  // our ruby exception type
TKernelParms KernelParms;    // Kernel Params

/*
 * raise an error and tell wich method failed with wich error code
 */
static void vacman_raise_error(char* method, int error_code) {
  aat_ascii error_message[100];
  AAL2GetErrorMsg (error_code, error_message);
  rb_raise(e_vacmanerror, "%s error %d: %s", method, error_code, error_message);
}


/*
 * convert a ruby hash to TDigipassBlob structure
 */
static void rbhash_to_digipass(VALUE token, TDigipassBlob* dpdata) {
  memset(dpdata, 0, sizeof(*dpdata));

  VALUE blob = rb_hash_aref(token, rb_str_new2("blob"));
  VALUE serial = rb_hash_aref(token, rb_str_new2("serial"));
  VALUE app_name = rb_hash_aref(token, rb_str_new2("app_name"));
  VALUE flag1 = rb_hash_aref(token, rb_str_new2("flags1"));
  VALUE flag2 = rb_hash_aref(token, rb_str_new2("flags2"));

  strcpy(dpdata->Blob, rb_string_value_cstr(&blob));
  strncpy(dpdata->Serial, rb_string_value_cstr(&serial), sizeof(dpdata->Serial));
  strncpy(dpdata->AppName, rb_string_value_cstr(&app_name), sizeof(dpdata->AppName));
  dpdata->DPFlags[0] = rb_fix2int(flag1);
  dpdata->DPFlags[1] = rb_fix2int(flag2);
}

static void digipass_to_rbhash(TDigipassBlob* dpdata, VALUE hash) {
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
 * Get library version and return it as an hash
 */
static VALUE vacman_library_version(VALUE module) {
  aat_ascii version[16];
  aat_int32 version_len = sizeof(version);

  aat_ascii bitness[4];
  aat_int32 bitness_len = sizeof(bitness);

  aat_ascii type[8];
  aat_int32 type_len = sizeof(type);

  aat_int32 result = AAL2GetLibraryVersion(version, &version_len, bitness,
      &bitness_len, type, &type_len);

  if (result != 0) {
    vacman_raise_error("AAL2GetLibraryVersion", result);
    return Qnil;
  }

  VALUE hash = rb_hash_new();
  rb_hash_aset(hash, rb_str_new2("version"), rb_str_new2(version));
  rb_hash_aset(hash, rb_str_new2("bitness"), rb_str_new2(bitness));
  rb_hash_aset(hash, rb_str_new2("type"),    rb_str_new2(type));

  return hash;
}


/*
 * generate a password
 * this will not work with all the dpx files available, it must be prepared for it
 */
static VALUE vacman_generate_password(VALUE module, VALUE token) {
  TDigipassBlob dpdata;

  rbhash_to_digipass(token, &dpdata);

  aat_ascii password[18];
  memset(password, 0, sizeof(password));

  aat_int32 result = AAL2GenPassword(&dpdata, &KernelParms, password, NULL);
  digipass_to_rbhash(&dpdata, token);

  if (result != 0) {
    vacman_raise_error("AAL2GenPassword", result);
    return Qnil;
  }

  return rb_str_new2(password);
}


/*
 * Properties names and IDs registry
 */
struct token_property {
  char *name;
  aat_int32 id;
};
static struct token_property token_properties[] = {
  {"token_model",                   TOKEN_MODEL                   },
  {"token_status",                  TOKEN_STATUS                  },
  {"use_count",                     USE_COUNT                     },
  {"last_time_used",                LAST_TIME_USED                },
  {"last_time_shift",               LAST_TIME_SHIFT               },
  {"time_based_algo",               TIME_BASED_ALGO               },
  {"event_based_algo",              EVENT_BASED_ALGO              },
  {"pin_supported",                 PIN_SUPPORTED                 },
  {"unlock_supported",              UNLOCK_SUPPORTED              },
  {"pin_ch_on",                     PIN_CH_ON                     },
  {"pin_change_enabled",            PIN_CH_ON                     },
  {"pin_len",                       PIN_LEN                       },
  {"pin_length",                    PIN_LEN                       },
  {"pin_min_len",                   PIN_MIN_LEN                   },
  {"pin_minimum_length",            PIN_MIN_LEN                   },
  {"pin_enabled",                   PIN_ENABLED                   },
  {"pin_ch_forced",                 PIN_CH_FORCED                 },
  {"pin_change_forced",             PIN_CH_FORCED                 },
  {"virtual_token_type",            VIRTUAL_TOKEN_TYPE            },
  {"virtual_token_grace_period",    VIRTUAL_TOKEN_GRACE_PERIOD    },
  {"virtual_token_remain_use",      VIRTUAL_TOKEN_REMAIN_USE      },
  {"last_response_type",            LAST_RESPONSE_TYPE            },
  {"error_count",                   ERROR_COUNT                   },
  {"event_value",                   EVENT_VALUE                   },
  {"last_event_value",              LAST_EVENT_VALUE              },
  {"sync_windows",                  SYNC_WINDOWS                  },
  {"primary_token_enabled",         PRIMARY_TOKEN_ENABLED         },
  {"virtual_token_supported",       VIRTUAL_TOKEN_SUPPORTED       },
  {"virtual_token_enabled",         VIRTUAL_TOKEN_ENABLED         },
  {"code_word",                     CODE_WORD                     },
  {"auth_mode",                     AUTH_MODE                     },
  {"ocra_suite",                    OCRA_SUITE                    },
  {"derivation_supported",          DERIVATION_SUPPORTED          },
  {"max_dtf_number",                MAX_DTF_NUMBER                },
  {"response_len",                  RESPONSE_LEN                  },
  {"response_length",               RESPONSE_LEN                  },
  {"response_format",               RESPONSE_FORMAT               },
  {"response_chk",                  RESPONSE_CHK                  },
  {"response_checksum",             RESPONSE_CHK                  },
  {"time_step",                     TIME_STEP                     },
  {"use_3des",                      TRIPLE_DES_USED               },
  {"triple_des_used",               TRIPLE_DES_USED               },
};

static size_t properties_count = sizeof(token_properties)/sizeof(struct token_property);

/*
 * Convert property name to property ID
 */
static long vacman_get_property_id(char *property_name) {
  for (int i = 0; i < properties_count; i++) {
    if (strcmp(property_name, token_properties[i].name) == 0) {
      return token_properties[i].id;
    }
  }

  rb_raise(e_vacmanerror, "Invalid property name `%s'", property_name);
  return 0;
}


/*
 * Get token properties
 */
static VALUE vacman_get_token_property(VALUE module, VALUE token, VALUE property) {
  TDigipassBlob dpdata;
  rbhash_to_digipass(token, &dpdata);

  aat_ascii value[64];
  aat_int32 property_id = vacman_get_property_id(StringValueCStr(property));
  aat_int32 result = AAL2GetTokenProperty(&dpdata, &KernelParms, property_id, value);

  if (result == 0) {
    return rb_str_new2(value);
  } else {
    vacman_raise_error("AAL2GetTokenProperty", result);
    return Qnil;
  }
}


/*
 * Set token properties
 */
static VALUE vacman_set_token_property(VALUE module, VALUE token, VALUE property, VALUE rbval) {
  TDigipassBlob dpdata;

  aat_int32 property_id = vacman_get_property_id(StringValueCStr(property));
  aat_int32 value = rb_fix2int(rbval);

  rbhash_to_digipass(token, &dpdata);

  aat_int32 result = AAL2SetTokenProperty(&dpdata, &KernelParms, property_id, value);

  digipass_to_rbhash(&dpdata, token);

  if (result == 0) {
    return Qtrue;
  } else {
    vacman_raise_error("AAL2SetTokenProperty", result);
    return Qnil;
  }
}


/*
 * verify password
 * this is the main usecase, check the use input for authentication
 */
static VALUE vacman_verify_password(VALUE module, VALUE token, VALUE password ) {
  TDigipassBlob dpdata;

  rbhash_to_digipass(token, &dpdata);

  aat_int32 result = AAL2VerifyPassword(&dpdata, &KernelParms, rb_string_value_cstr(&password), 0);

  digipass_to_rbhash(&dpdata, token);

  if (result == 0)
    return Qtrue;
  else {
    vacman_raise_error("AAL2VerifyPassword", result);
    return Qnil;
  }
}



/*
 * Import a .DPX file containing token seeds and initialisation values.
 *
 * Pass the pre-shared key to validate it as the second argument. Or if
 * you don't have the key, replace the DC line with one from a file you
 * know the key. Yes.
 */
static VALUE vacman_import(VALUE module, VALUE filename, VALUE key) {
  TDPXHandle dpx_handle;
  aat_int16 appl_count;
  aat_ascii appl_names[13*8];
  aat_int16 token_count;

  aat_int32 result = AAL2DPXInit(&dpx_handle, rb_string_value_cstr(&filename), rb_string_value_cstr(&key),
      &appl_count, appl_names, &token_count);

  if (result != 0) {
    vacman_raise_error("AAL2DPXInit", result);
    return Qnil;
  }

  aat_ascii sw_out_serial_No[22+1];
  aat_ascii sw_out_type[5+1];
  aat_ascii sw_out_authmode[2+1];
  TDigipassBlob dpdata;

  VALUE list = rb_ary_new();

  while (1) {
    result = AAL2DPXGetToken(&dpx_handle,
        &KernelParms,
        appl_names,
        sw_out_serial_No,
        sw_out_type,
        sw_out_authmode,
        &dpdata);


    if (result < 0) {
      vacman_raise_error("AAL2DPXGetToken", result);
      return Qnil;
    }
    if (result == 107) break;

    VALUE hash = rb_hash_new();

    digipass_to_rbhash(&dpdata, hash);

    rb_ary_push(list, hash);
  }

  AAL2DPXClose(&dpx_handle);

  return list;
}

struct kernel_property {
  char *name;
  aat_int32 *value;
  aat_int32 deflt;
};
static struct kernel_property kernel_properties[] = {
  { "ITimeWindow",    &KernelParms.ITimeWindow,    30  },  // Identification Window size in time steps
  { "STimeWindow",    &KernelParms.STimeWindow,    24  },  // Signature Window size in secs
  { "DiagLevel",      &KernelParms.DiagLevel,      0   },  // Requested Diagnostic Level
  { "GMTAdjust",      &KernelParms.GMTAdjust,      0   },  // GMT Time adjustment to perform
  { "CheckChallenge", &KernelParms.CheckChallenge, 0   },  // Verify Challenge Corrupted (mandatory for Gordian)
  { "IThreshold",     &KernelParms.IThreshold,     3   },  // Identification Error Threshold
  { "SThreshold",     &KernelParms.SThreshold,     1   },  // Signature Error Threshold
  { "ChkInactDays",   &KernelParms.ChkInactDays,   0   },  // Check Inactive Days
  { "DeriveVector",   &KernelParms.DeriveVector,   0   },  // Vector used to make Data Encryption unique
  { "SyncWindow",     &KernelParms.SyncWindow,     2   },  // Synchronisation Time Window (h)
  { "OnLineSG",       &KernelParms.OnLineSG,       2   },  // On line signature
  { "EventWindow",    &KernelParms.EventWindow,    100 },  // Event Window size in nbr of iterations
  { "HSMSlotId",      &KernelParms.HSMSlotId,      0   },  // HSM Slot id uses to store DB and Transport Key
};
static int kernel_properties_count = sizeof(kernel_properties)/sizeof(struct kernel_property);

/*
 * Set kernel parameter
 */
static VALUE vacman_set_kernel_param(VALUE module, VALUE paramname, VALUE rbval) {
  char *name = StringValueCStr(paramname);
  int value  = rb_fix2int(rbval);

  for (int i = 0; i < kernel_properties_count; i++) {
    if (strcmp(name, kernel_properties[i].name) == 0) {
      *kernel_properties[i].value = value;
      return Qtrue;
    }
  }

  rb_raise(e_vacmanerror, "Invalid kernel param %s", name);
  return Qnil;
}


/*
 * Get kernel parameter
 */
static VALUE vacman_get_kernel_param(VALUE module, VALUE paramname) {
  char *name = StringValueCStr(paramname);

  for (int i = 0; i < kernel_properties_count; i++) {
    if (strcmp(name, kernel_properties[i].name) == 0) {
      return LONG2FIX(*kernel_properties[i].value);
    }
  }

  rb_raise(e_vacmanerror, "Invalid kernel param %s", name);
  return Qnil;
}

/*
 * Init the kernel parameters, with their defaults
 */
static void init_kernel_params() {
  memset(&KernelParms, 0, sizeof(TKernelParms));

  KernelParms.ParmCount = 19; /* Number of valid parameters in this list */

  for (int i = 0; i < kernel_properties_count; i++) {
    *kernel_properties[i].value = kernel_properties[i].deflt;
  }
}


/*
 * rubys entry point to load the extension
 */
void Init_vacman_controller(void) {
  VALUE vacman_module = rb_define_module("VacmanLowLevel");

  e_vacmanerror = rb_define_class("VacmanError", rb_eStandardError);
  init_kernel_params();

  rb_define_singleton_method(vacman_module, "version",            vacman_library_version, 0);

  rb_define_singleton_method(vacman_module, "import",             vacman_import, 2);

  rb_define_singleton_method(vacman_module, "generate_password",  vacman_generate_password, 1);
  rb_define_singleton_method(vacman_module, "verify_password",    vacman_verify_password, 2);

  rb_define_singleton_method(vacman_module, "get_kernel_param",   vacman_get_kernel_param, 1);
  rb_define_singleton_method(vacman_module, "set_kernel_param",   vacman_set_kernel_param, 2);

  rb_define_singleton_method(vacman_module, "get_token_property", vacman_get_token_property, 2);
  rb_define_singleton_method(vacman_module, "set_token_property", vacman_set_token_property, 3);
}
