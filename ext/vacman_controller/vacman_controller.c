#include <ruby.h>
#include <string.h>
#include <aal2sdk.h>


static VALUE e_vacmanerror;													// our ruby exception type
TKernelParms   KernelParms;													// Kernel Params

/*
 * raise an error and tell wich method failed with wich error code
 */
void raise_error(char* method, int error_code) {
	char buffer[256];
	AAL2GetErrorMsg (error_code, buffer);
  rb_raise(e_vacmanerror, buffer);
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
 * generate a password 
 * this will not work with all the dpx files available, it must be prepared for it
 */
static VALUE vacman_generate_password(VALUE module, VALUE data ) {
  int result;
  TDigipassBlob dpdata;
  
  rbhash_to_digipass(data, &dpdata);  

  char buffer[256];
  memset(buffer, 0, sizeof(buffer));
  result = AAL2GenPassword(&dpdata, &KernelParms, buffer, 0);
  digipass_to_rbhash(&dpdata, data);
    
  if (result != 0) {
    raise_error("AAL2GenPassword", result);
    return;
  }

  return rb_str_new2(buffer);
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
                                 &appl_count, &appl_names, &token_count);

  if (result != 0) {
  	raise_error("AAL2DPXInit", result);
    return;
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
	    return;
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
static void vacman_set_kernal_param(VALUE module, VALUE paramname, VALUE rbval) {
  char* name = rb_string_value_cstr(&paramname);
  int val = rb_fix2int(rbval);
  if (strcmp(name, "itimewindow") == 0)
    return KernelParms.ITimeWindow = val;
  else {
    char buffer[256];
    sprintf(buffer, "invalid kernal param %s", name);
    rb_raise(e_vacmanerror, buffer);
    return;
  }
}


/*
 * init the kernal parameters, this is all static up to now, we can later
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

  rb_define_singleton_method(vacman_module, "import", vacman_import, 2);
  rb_define_singleton_method(vacman_module, "generate_password", vacman_generate_password, 1);
  rb_define_singleton_method(vacman_module, "verify_password", vacman_verify_password, 2);
  rb_define_singleton_method(vacman_module, "set_kernal_param", vacman_set_kernal_param, 2);
}