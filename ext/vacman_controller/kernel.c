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
 * Vacman Controller kernel properties
 */
struct kernel_property {
  const char *name;
  aat_int32 *value;
  aat_int32 deflt;
};
static struct kernel_property vacman_kernel_properties[] = {
  { "ITimeWindow",    &g_KernelParms.ITimeWindow,    30  },  // Identification Window size in time steps
  { "STimeWindow",    &g_KernelParms.STimeWindow,    24  },  // Signature Window size in secs
  { "DiagLevel",      &g_KernelParms.DiagLevel,      0   },  // Requested Diagnostic Level
  { "GMTAdjust",      &g_KernelParms.GMTAdjust,      0   },  // GMT Time adjustment to perform
  { "CheckChallenge", &g_KernelParms.CheckChallenge, 0   },  // Verify Challenge Corrupted (mandatory for Gordian)
  { "IThreshold",     &g_KernelParms.IThreshold,     3   },  // Identification Error Threshold
  { "SThreshold",     &g_KernelParms.SThreshold,     1   },  // Signature Error Threshold
  { "ChkInactDays",   &g_KernelParms.ChkInactDays,   0   },  // Check Inactive Days
  { "DeriveVector",   &g_KernelParms.DeriveVector,   0   },  // Vector used to make Data Encryption unique
  { "SyncWindow",     &g_KernelParms.SyncWindow,     2   },  // Synchronisation Time Window (h)
  { "OnLineSG",       &g_KernelParms.OnLineSG,       2   },  // On line signature
  { "EventWindow",    &g_KernelParms.EventWindow,    100 },  // Event Window size in nbr of iterations
  { "HSMSlotId",      &g_KernelParms.HSMSlotId,      0   },  // HSM Slot id uses to store DB and Transport Key
};
static size_t vacman_kernel_properties_count = sizeof(vacman_kernel_properties)/sizeof(struct kernel_property);

/*
 * Initialise the kernel parameters with their defaults
 */
void vacman_kernel_init_params() {
  memset(&g_KernelParms, 0, sizeof(g_KernelParms));

  g_KernelParms.ParmCount = 19; /* Number of valid parameters in this list */

  for (size_t i = 0; i < vacman_kernel_properties_count; i++) {
    *vacman_kernel_properties[i].value = vacman_kernel_properties[i].deflt;
  }
}


/*
 * Get kernel parameter names
 */
VALUE vacman_kernel_get_property_names() {
  VALUE ret = rb_ary_new();

  for (size_t i = 0; i < vacman_kernel_properties_count; i++) {
    const char *name = vacman_kernel_properties[i].name;
    rb_ary_push(ret, rb_str_new2(name));
  }

  return ret;
}

/*
 * Get kernel parameter
 */
VALUE vacman_kernel_get_param(VALUE module, VALUE paramname) {
  char *name = StringValueCStr(paramname);

  for (size_t i = 0; i < vacman_kernel_properties_count; i++) {
    if (strcmp(name, vacman_kernel_properties[i].name) == 0) {
      return LONG2FIX(*vacman_kernel_properties[i].value);
    }
  }

  rb_raise(e_VacmanError, "Invalid kernel param %s", name);
  return Qnil;
}

/*
 * Set kernel parameter
 */
VALUE vacman_kernel_set_param(VALUE module, VALUE paramname, VALUE rbval) {
  char *name = StringValueCStr(paramname);
  int value  = rb_fix2int(rbval);

  for (size_t i = 0; i < vacman_kernel_properties_count; i++) {
    if (strcmp(name, vacman_kernel_properties[i].name) == 0) {
      *vacman_kernel_properties[i].value = value;
      return Qtrue;
    }
  }

  rb_raise(e_VacmanError, "Invalid kernel param %s", name);
  return Qnil;
}
