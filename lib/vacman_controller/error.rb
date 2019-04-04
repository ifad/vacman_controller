module VacmanController

  # Represents a Vacman Controller Library error.
  #
  class Error < StandardError
    # The AAL2 library method that errored
    attr_reader :library_method

    # The error code returned by the AAL2 library
    attr_reader :error_code

    # The error message retrieved by the AAL2 library
    attr_reader :error_message
  end

end
