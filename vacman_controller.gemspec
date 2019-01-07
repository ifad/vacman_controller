Gem::Specification.new do |s|
  s.name        = 'vacman_controller'
  s.version     = '0.1.2'
  s.date        = '2019-01-07'
  s.summary     = "Access to the vacman controller library"
  s.description = "Authenticate user via vacman controller"
  s.authors     = ["Marcus Lankenau", "Marcello Barnaba"]
  s.email       = ["marcus.lankenau@gmail.com", "marcello.barnaba@gmail.com"]
  s.files       = Dir.glob('lib/**/*.rb') + Dir.glob('ext/**/*.{c,h,rb}')
  s.homepage    = 'http://github.com/ifad/vacman_controller'
  s.extensions  = ['ext/vacman_controller/extconf.rb']
  s.add_development_dependency 'rake-compiler'
end
