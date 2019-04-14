module VacmanController
  class Token

    class Properties
      class << self
        # Gets the available token property names
        #
        def names
          @_names ||= VacmanController::LowLevel.token_property_names.freeze
        end
      end


      def initialize(token)
        @token = token
      end


      def inspect
        all.inspect
      end


      def all
        self.class.names.inject({}) do |h, name|
          h.update(name => (self[name] rescue "ERROR: #$!"))
        end
      end
      alias to_h all


      # Get a Token property
      #
      # == Parameters:
      #
      # property::
      #   the property name. See +Token::Properties.names+
      #
      def [](name)
        name  = name.to_s
        value = VacmanController::LowLevel.get_token_property(@token.to_h, name)

        read_cast(name, value)
      end


      # Set a Token property
      #
      # == Parameters:
      #
      # property::
      #   the property name. See +Token.property_names+
      #
      # value::
      #   the property value. The AAL2 library accepts only values
      #   convertible to integer. For symmetry with +[]+, boolean
      #   values are converted to the appropriate integer.
      #
      # see::
      #   +write_cast!+
      #
      def []=(name, value)
        name  = name.to_s
        value = write_cast!(name, value)

        VacmanController::LowLevel.set_token_property(@token.to_h, name, value)
      end

      protected
        #
        def write_cast!(property, value)
          case property
            # Bounded integer values
          when 'last_time_used'
            write_check_bounds! property, value,
              [631152000, 2147483647]

          when 'last_time_shift'
            write_check_bounds! property, value,
              [-100_000, 100_000]

          when 'pin_enabled'
            value ? 1 : 2

          when 'pin_change_forced'
            if value
              1
            else
              raise VacmanController::Error,
                "Token property #{property} cannot be set to #{value.inspect}"
            end

          when 'pin_min_len', 'pin_minimum_length'
            write_check_bounds! property, value,
              [3, 8]

          when 'virtual_token_grace_period'
            write_check_bounds! property, value,
              [1, 364]

          when 'virtual_token_remain_use'
            write_check_bounds! property, value,
              [0, 254]

          when 'error_count'
            if value == 0
              value
            else
              raise VacmanController::Error,
                "Token property #{property} cannot be set to #{value.inspect}"
            end

          when 'event_value'
            write_check_bounds! property, value,
              [0, 4_294_967_294]

          when 'token_status'
            case value
            when :disabled     then 0
            when :primary_only then 1
            when :backup_only  then 2
            when :enabled      then 3
            else
              raise VacmanController::Error,
                "Token property #{property} cannot be set to #{value.inspect}"
            end

          else
            raise VacmanController::Error,
              "Invalid or read-only property: #{property}"
          end
        end

        # The library also does these checks, but we want to present
        # custom error message to the caller.
        #
        def write_check_bounds!(property, value, bounds)
          min, max = bounds

          if value < min || value > max
            raise VacmanController::Error,
              "Invalid #{property} value provided: #{value}. " \
              "Must be between greater than #{min} and less than #{max}."
          end

          value
        end


        # Maps the given property value to a Ruby value for:
        #
        # * Integers
        # * Booleans
        # * Dates
        # * Enumerations (Symbols)
        #
        def read_cast(property, value)
          # Short-circuit on 'NA'
          return nil if value == 'NA' or value == 'DISABLE'

          case property
          when  # Integer values
            'use_count',
            'pin_len',
            'pin_length',
            'pin_min_len',
            'pin_minimum_length',
            'last_time_shift',
            'virtual_token_remain_use',
            'error_count',
            'event_value',
            'last_event_value',
            'max_dtf_number',
            'response_len',
            'response_length',
            'time_step'
            value.to_i

          when # Boolean values
            'time_based_algo',
            'event_based_algo',
            'pin_supported',
            'unlock_supported',
            'pin_ch_on',
            'pin_change_enabled',
            'pin_enabled',
            'pin_ch_forced',
            'pin_change_forced',
            'sync_windows',
            'primary_token_enabled',
            'virtual_token_supported',
            'virtual_token_enabled',
            'derivation_supported',
            'response_chk',
            'response_checksum',
            'triple_des_used',
            'use_3des'

            case value
            when 'YES' then true
            when 'NO' then false
            end

          when # Date/time values
            'last_time_used',
            'virtual_token_grace_period'

            # AAL2 returns UTC values, we add the timezone.
            Time.strptime("#{value} UTC", '%a %b %d %H:%M:%S %Y %Z')

          when
            'auth_mode'

            case value
            when 'RO' then :response_only
            when 'SG' then :signature_application
            when 'CR' then :challenge_response
            when 'MM' then :multi_mode
            when 'UL' then :unlock_v2
            else
              value
            end

          else # String values

            value
          end
        end

      private
        # Exposes a getter and setter method for each
        # property name. Delegates error handling for
        # invalid properties to +[]+ and +[]=+.
        #
        def method_missing(name, *args, &block)
          prop, setter = name.to_s.match(/\A(.+?)(=)?\Z/).values_at(1, 2)

          if setter
            self[prop] = args.first
          else
            self[prop]
          end
        end
    end

  end
end
