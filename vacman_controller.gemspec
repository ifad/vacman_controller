Gem::Specification.new do |s|
  s.name        = 'vacman_controller'
  s.version     = '0.1.0'
  s.date        = '2013-01-07'
  s.summary     = "Access to the vacman controller library"
  s.description = "Authenticate user via vacman controller"
  s.authors     = ["Marcus Lankenau"]
  s.email       = 'marcus.lankenau@gmail.com'
  s.files       = Dir.glob('lib/**/*.rb') + Dir.glob('ext/**/*.{c,h,rb}')
  s.homepage    = 'http://github.com/mlankenau/vacman_controller'
  s.extensions  = ['ext/vacman_controller/extconf.rb']
end
