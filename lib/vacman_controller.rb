require 'vacman_controller/low_level'
require 'vacman_controller/token'
require 'vacman_controller/kernel'
require 'vacman_controller/error'

# Wraps VACMAN Controller functionality for Ruby.
#
module VacmanController

  VERSION = '0.5.0'

  class << self
    # Imports a .dpx file containing the token key material.
    #
    # == Parameters:
    #
    # filename::
    #   The path of the .dpx file to load
    #
    # key::
    #   The transport key to decrypt the dpx file
    #
    # == Returns:
    # An Array of Ruby Hashes. Each Hash contains the following keys:
    #
    #   serial:   the serial number of the token
    #   blob:     the blob containing some secret magic data
    #   app_name: the application name (the security method)
    #   flags1:   flags
    #   flags2:   flags
    #
    # This is only for low-level usage. For a Ruby API, look at
    # +VacmanController::Token.import+.
    #
    def import(filename, key)
      VacmanController::LowLevel.import(filename, key)

    rescue VacmanController::Error => e
      # We handle two undocumented error codes here
      case e.error_code
      when -15
        raise VacmanController::Error, "#{e.library_method} error #{e.error_code}: invalid transport key"

      when -20
        raise VacmanController::Error, "#{e.library_method} error #{e.error_code}: cannot open DPX file"

      else
        raise # Sorry, I did my best.

      end
    end

    # Returns the +Kernel+ module
    #
    def kernel
      VacmanController::Kernel
    end
  end

end
