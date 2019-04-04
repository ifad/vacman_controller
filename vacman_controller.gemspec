Gem::Specification.new do |s|
  s.name        = 'vacman_controller'
  s.version     = '0.5.0'
  s.date        = '2019-04-04'
  s.summary     = "Ruby layer to access VASCO Vacman Controller functions"
  s.description = "Expose the AAL2 SDK API via a set of Ruby classes optimised for developer happiness"
  s.authors     = ["Marcus Lankenau", "Marcello Barnaba"]
  s.email       = ["marcus.lankenau@gmail.com", "marcello.barnaba@gmail.com"]
  s.files       = Dir.glob('lib/**/*.rb') + Dir.glob('ext/**/*.{c,h,rb}')
  s.homepage    = 'https://github.com/ifad/vacman_controller'
  s.extensions  = ['ext/vacman_controller/extconf.rb']

  s.add_development_dependency 'rake-compiler'
  s.add_development_dependency 'rspec'
end
