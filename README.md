VASCO VACMAN Controller
=======================

VACMAN Controller is VASCO's implementation of OTP physical and virtual
devices, revolving around the AAL2 library.

This gem contains a very thin wrapper around AAL2 and allows to parse DPX
files, generate OTPs and verify them.

Installation
------------

Get Vacman Controller library from the [TrustBuilder
repo](https://repository.trustbuilder.io/head/trustbuilder/custom/), download
the `aal2sdk-*.rpm` and place its contents in `/opt/vasco`. The Ruby extension
looks for `/opt/vasco/VACMAN_Controller-*`

Add to your application Gemfile

    gem 'vacman_controller'

To run specs download the sources and execute

    rake
