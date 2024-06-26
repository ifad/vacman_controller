require 'mkmf'

def find_vacman_controller
  Dir.glob('/opt/vasco/VACMAN_Controller-*').sort.reverse.first
end

VACMAN_CONTROLLER = ENV['VACMAN_PATH'] || find_vacman_controller

if VACMAN_CONTROLLER
  puts "Using VACMAN controller from #{VACMAN_CONTROLLER}"
else
  puts "No VASCO Vacman controller found in /opt/vasco"
  exit 1
end

append_cflags "-I#{VACMAN_CONTROLLER}/include -fcommon -Wall -std=c99 -Wno-declaration-after-statement"
append_ldflags "-L#{VACMAN_CONTROLLER}/lib -laal2sdk -Wl,-rpath #{VACMAN_CONTROLLER}/lib"

if find_library('aal2sdk', 'AAL2DPXInit', "#{VACMAN_CONTROLLER}/lib")
  create_makefile('vacman_controller/vacman_low_level')
else
  puts "No libaal2sdk found"
  exit 1
end
