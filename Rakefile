require "bundler/gem_tasks"
require "rspec/core/rake_task"

RSpec::Core::RakeTask.new(:spec) do |spec|
  spec.rspec_opts = '-f doc'
end

require "rake/extensiontask"

task build: :compile

Rake::ExtensionTask.new('vacman_controller') do |ext|
  ext.lib_dir = 'lib/vacman_controller'
  ext.name    = 'vacman_low_level'
end

task default: [:clobber, :compile, :spec]

require 'code_counter/engine'
desc 'Print code statistics'
task :stats do
  puts CodeCounter::Engine.new.to_s
end
