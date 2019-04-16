FROM centos:7

WORKDIR /build
COPY . .

# Update packages
#
RUN yum -y update

# Install dependencies
#
RUN yum -y install \
  --setopt=deltarpm=0 \
  --setopt=install_weak_deps=false \
  --setopt=tsflags=nodocs \
  git \
  gcc \
  make \
  centos-release-scl

# Install Ruby 2.3
#
RUN yum -y install \
  --setopt=deltarpm=0 \
  --setopt=install_weak_deps=false \
  --setopt=tsflags=nodocs \
  rh-ruby23 \
  rh-ruby23-ruby-devel \
  rh-ruby23-rubygems

# Install bundler and bundle dependencies
#
RUN source /opt/rh/rh-ruby23/enable; \
  ruby -v; \
  gem install bundler; \
  bundle -v; \
  bundle install --path .bundle
