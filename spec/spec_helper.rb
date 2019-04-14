# See http://rubydoc.info/gems/rspec-core/RSpec/Core/Configuration
#
require 'simplecov'
SimpleCov.start do
  add_filter '.bundle'
end

require 'byebug'

require 'rspec'
require 'vacman_controller'
