#include <ruby.h>
#include <string.h>
#include <aal2sdk.h>

static VALUE e_vacmanerror;  // our ruby exception type
TKernelParms   KernelParms;  // Kernel Params

/*
 * raise an error and tell wich method failed with wich error code
 */
void raise_error(char* method, int error_code) {
  char buffer[256];
  AAL2GetErrorMsg (error_code, buffer);
  rb_raise(e_vacmanerror, "%s: %s", method, buffer);
}


/*
 * convert a ruby hash to TDigipassBlob structure
 */
static void rbhash_to_digipass(VALUE data, TDigipassBlob* dpdata) {
  memset(dpdata, 0, sizeof(dpdata));

  VALUE blob = rb_hash_aref(data, rb_str_new2("blob"));
  VALUE serial = rb_hash_aref(data, rb_str_new2("serial"));
  VALUE app_name = rb_hash_aref(data, rb_str_new2("app_name"));
  VALUE flag1 = rb_hash_aref(data, rb_str_new2("flags1"));
  VALUE flag2 = rb_hash_aref(data, rb_str_new2("flags2"));

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
    raise_error("AAL2GetLibraryVersion", result);
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
static VALUE vacman_generate_password(VALUE module, VALUE data ) {
  int result;
  TDigipassBlob dpdata;

  rbhash_to_digipass(data, &dpdata);

  char buffer[256];
  memset(buffer, 0, sizeof(buffer));
  result = AAL2GenPassword(&dpdata, &KernelParms, buffer, NULL);
  digipass_to_rbhash(&dpdata, data);

  if (result != 0) {
    raise_error("AAL2GenPassword", result);
    return Qnil;
  }

  return rb_str_new2(buffer);
}

/*
 * Get token properties
 */
static char *token_property_names[] = {
  "token_model",
  "use_count",
  "last_time_used",
  "last_time_shift",
  "time_based_algo",
  "event_based_algo",
  "pin_supported",
  "unlock_supported",
  "pin_change_enabled",
  "pin_length",
  "pin_minimum_length",
  "pin_enabled",
  "pin_change_forced",
  "virtual_token_type",
  "virtual_token_grace_period",
  "virtual_token_remain_use",
  "last_response_type",
  "error_count",
  "event_value",
  "last_event_value",
  "sync_windows",
  "primary_token_enabled",
  "virtual_token_supported",
  "virtual_token_enabled",
  "code_word",
  "auth_mode",
  "ocra_suite",
  "derivation_supported",
  "max_dtf_number",
  "response_length",
  "response_format",
  "response_checksum",
  "time_step",
  "use_3des",
};

static long token_property_ids[] = {
  TOKEN_MODEL,
  USE_COUNT,
  LAST_TIME_USED,
  LAST_TIME_SHIFT,
  TIME_BASED_ALGO,
  EVENT_BASED_ALGO,
  PIN_SUPPORTED,
  UNLOCK_SUPPORTED,
  PIN_CH_ON,
  PIN_LEN,
  PIN_MIN_LEN,
  PIN_ENABLED,
  PIN_CH_FORCED,
  VIRTUAL_TOKEN_TYPE,
  VIRTUAL_TOKEN_GRACE_PERIOD,
  VIRTUAL_TOKEN_REMAIN_USE,
  LAST_RESPONSE_TYPE,
  ERROR_COUNT,
  EVENT_VALUE,
  LAST_EVENT_VALUE,
  SYNC_WINDOWS,
  PRIMARY_TOKEN_ENABLED,
  VIRTUAL_TOKEN_SUPPORTED,
  VIRTUAL_TOKEN_ENABLED,
  CODE_WORD,
  AUTH_MODE,
  OCRA_SUITE,
  DERIVATION_SUPPORTED,
  MAX_DTF_NUMBER,
  RESPONSE_LEN,
  RESPONSE_FORMAT,
  RESPONSE_CHK,
  TIME_STEP,
  TRIPLE_DES_USED,
};
static size_t properties_count = sizeof(token_property_names)/sizeof(char*);

static long vacman_get_property_id(char *property_name) {
  for (int i = 0; i < properties_count; i++) {
    if (strcmp(property_name, token_property_names[i]) == 0) {
      return token_property_ids[i];
    }
  }

  rb_raise(e_vacmanerror, "Invalid property name `%s'", property_name);
  return 0;
}

static VALUE vacman_get_token_property(VALUE module, VALUE token, VALUE property) {
  TDigipassBlob dpdata;
  rbhash_to_digipass(token, &dpdata);

  char buffer[64];
  long property_id = vacman_get_property_id(StringValueCStr(property));
  aat_int32 result = AAL2GetTokenProperty(&dpdata, &KernelParms, property_id, buffer);

  if (result == 0) {
    return rb_str_new2(buffer);
  } else {
    raise_error("AAL2GetTokenProperty", result);
    return Qnil;
  }
}


/*
 * verify password
 * this is the main usecase, check the use input for authentication
 */
static VALUE vacman_verify_password(VALUE module, VALUE data, VALUE password ) {
  int result;
  TDigipassBlob dpdata;

  rbhash_to_digipass(data, &dpdata);

  char buffer[256];
  result = AAL2VerifyPassword(&dpdata, &KernelParms, rb_string_value_cstr(&password), 0);

  digipass_to_rbhash(&dpdata, data);

  if (result == 0)
    return Qtrue;
  else
    raise_error("AAL2VerifyPassword", result);
}



/*
 * do import a dpx file containing
 */
static VALUE vacman_import(VALUE module, VALUE filename, VALUE key) {
  TDPXHandle dpx_handle;
  aat_int16 appl_count;
  aat_ascii appl_names[13*8];
  aat_int16 token_count;

  aat_int32 result = AAL2DPXInit(&dpx_handle, rb_string_value_cstr(&filename), rb_string_value_cstr(&key),
      &appl_count, appl_names, &token_count);

  if (result != 0) {
    raise_error("AAL2DPXInit", result);
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
      raise_error("AAL2DPXGetToken", result);
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


/*
 * set kernel parameters
 */
static VALUE vacman_set_kernel_param(VALUE module, VALUE paramname, VALUE rbval) {
  char* name = rb_string_value_cstr(&paramname);
  int val = rb_fix2int(rbval);

  if (strcmp(name, "itimewindow") == 0) {
    KernelParms.ITimeWindow = val;
    return rbval;

  } else {
    char buffer[256];
    sprintf(buffer, "invalid kernel param %s", name);
    rb_raise(e_vacmanerror, "%s", buffer);
    return Qnil;
  }
}


/*
 * init the kernel parameters, this is all static up to now, we can later
 * expose this via ruby methods if neccessary
 */
void init_kernel_params() {
  memset(&KernelParms, 0, sizeof(TKernelParms));
  KernelParms.ParmCount     = 19;     /* Number of valid parameters in this list */
  KernelParms.ITimeWindow   = 30;     /* Identification Window size in time steps*/
  KernelParms.STimeWindow   = 24;     /* Signature Window size in secs */
  KernelParms.DiagLevel     = 0;      /* Requested Diagnostic Level */
  KernelParms.GMTAdjust     = 0;      /* GMT Time adjustment to perform */
  KernelParms.CheckChallenge= 0;      /* Verify Challenge Corrupted (mandatory for Gordian) */
  KernelParms.IThreshold    = 3;      /* Identification Error Threshold */
  KernelParms.SThreshold    = 1;      /* Signature Error Threshold */
  KernelParms.ChkInactDays  = 0;      /* Check Inactive Days */
  KernelParms.DeriveVector  = 0;      /* Vector used to make Data Encryption unique     */
  KernelParms.SyncWindow    = 2;      /* Synchronisation Time Window (h)            */
  KernelParms.OnLineSG      = 1;      /* On line  Signature                 */
  KernelParms.EventWindow   = 100;    /* Event Window size in nbr of iterations       */
  KernelParms.HSMSlotId     = 0;      /* HSM Slot id uses to store DB and Transport Key   */
}


/*
 * rubys entry point to load the extension
 */
void Init_vacman_controller(void) {
  /* assume we haven't yet defined Hola */
  VALUE vacman_module = rb_define_module("VacmanLowLevel");

  e_vacmanerror = rb_define_class("VacmanError", rb_eStandardError);
  init_kernel_params();

  rb_define_singleton_method(vacman_module, "version", vacman_library_version, 0);
  rb_define_singleton_method(vacman_module, "import", vacman_import, 2);
  rb_define_singleton_method(vacman_module, "generate_password", vacman_generate_password, 1);
  rb_define_singleton_method(vacman_module, "verify_password", vacman_verify_password, 2);
  rb_define_singleton_method(vacman_module, "set_kernel_param", vacman_set_kernel_param, 2);
  rb_define_singleton_method(vacman_module, "get_token_property", vacman_get_token_property, 2);
}
