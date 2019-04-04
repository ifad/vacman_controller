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
        value = VacmanController::LowLevel.get_token_property(@token.to_h, property, name)

        cast(name, value)
      end


      # Set a Token property
      #
      # == Parameters:
      #
      # property::
      #   the property name. See +Token.property_names+
      #
      # value::
      #   the property value. Only values convertible to integer are
      #   supported.
      #
      def []=(name, value)
        name  = name.to_s
        value = value.to_i

        check_bounded_property!(name, value)

        VacmanController::LowLevel.set_token_property(@token.to_h, name, value)
      end


      protected
        def bounded_property?(name)
          PROPERTY_BOUNDS.key?(name)
        end

        # The library also does these checks, but we want to present
        # custom error message to the caller.
        #
        def check_bounded_property!(name, value)
          return unless bounded_property?(name)

          min, max = PROPERTY_BOUNDS.fetch(name)

          if value < min || value > max
            raise VacmanError,
              "Invalid #{property} value provided: #{value}. " \
              "Must be between greater than #{min} and less than #{max}."
          end

          true
        end

        PROPERTY_BOUNDS = {
          'last_time_used'             => [ 631152000, 2147483647 ],

          'last_time_shift'            => [ -100_000, 100_000 ],

          'pin_min_length'             => [ 3, 8 ],
          'pin_minimum_length'         => [ 3, 8 ],

          'virtual_token_grace_period' => [ 1, 364 ],

          'virtual_token_remain_use'   => [ 0, 254 ],

          'event_value'                => [ 0, 4_294_967_294 ],
        }

        def cast(property, value)

          # Short-circuit on 'NA'
          return nil if value == 'NA'

          case property
          when # Integer values
            'use_count',
            'pin_len',
            'pin_length',
            'pin_min_len',
            'pin_min_length',
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
            'triple_des_used'

            value == 'YES'

          when # Date/time values
            'last_time_used',
            'virtual_token_grace_period',

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
