module VacmanController

  module Kernel
    class << self
      # Returns the library version as an hash
      #
      def version
        @_version = VacmanController::LowLevel.library_version
      end


      # Gets the available kernel property names
      #
      def property_names
        @_property_names ||= VacmanController::LowLevel.kernel_property_names.freeze
      end


      # Shows the kernel version, bitness, type and parameters in your
      # development console.
      #
      def inspect
        "#<#{self.name} version=#{version['version'].inspect} bitness=#{version['bitness'].inspect} "\
        "type=#{version['type'].inspect} parameters=#{all.inspect}>>"
      end


      # Returns all configured parameters
      #
      def all
        property_names.inject({}) do |h, name|
          h.update(name => (self[name] rescue "ERROR: #$!"))
        end
      end
      alias to_h all


      # Get a Kernel property, that is a runtime parameter of Vacman Controller,
      # stored in the TKernelParms structure.
      #
      # == Parameters:
      # name::
      #   The param name. See +property_names+ for a list of available property
      #   names.
      #
      def [](name)
        VacmanController::LowLevel.get_kernel_param(name)
      end


      # Set a Kernel property.
      #
      # == Parameters:
      # name::
      #   the param name. See +property_names+ for a list of available property
      #   names. The Kernel parameters are all int32 properties.
      #
      # val::
      #   the integer value
      #
      def []=(name, val)
        Mutex.synchronize do
          VacmanController::LowLevel.set_kernel_param(name, val)
        end
      end

      Mutex = Thread::Mutex.new
    end
  end

end
