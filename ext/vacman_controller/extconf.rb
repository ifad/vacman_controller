require 'mkmf'

VACMAN_CONTROLLER = ENV['VACMAN_PATH'] || '/opt/vasco/VACMAN_Controller-3.15.1'

append_cflags "-I#{VACMAN_CONTROLLER}/include"
append_ldflags "-L#{VACMAN_CONTROLLER}/lib -laal2sdk -Wl,-rpath #{VACMAN_CONTROLLER}/lib"

if find_library('aal2sdk', 'AAL2DPXInit', "#{VACMAN_CONTROLLER}/lib")
  create_makefile('vacman_controller/vacman_controller')
 else
  puts "No libaal2sdk found"
  exit 1
end
