# Wee need unstable for now because of check version
FROM debian:unstable

ENV HOME /root

RUN \
  apt-get update && \
  apt-get -y upgrade && \
  apt-get -y install --no-install-recommends \
    autoconf autoconf-archive automake libtool \
    make pkg-config gcc gperf \
    git ca-certificates \
    libevent-dev libczmq-dev libmsgpack-dev libconfig-dev \
    check cppcheck valgrind lcov \
    zip xz-utils \
    && \
  apt-get clean

# Compile logc{,-libs}
RUN \
  git clone --branch "v0.3.0" "https://gitlab.nic.cz/turris/logc.git" && \
  cd logc && \
  ./bootstrap && ./configure --prefix=/usr && make install && \
  cd .. && \
  rm -rf logc
RUN \
  git clone --branch "v0.1.0" "https://gitlab.nic.cz/turris/logc-libs.git" && \
  cd logc-libs && \
  ./bootstrap && ./configure --prefix=/usr && make install && \
  cd .. && \
  rm -rf logc-libs


CMD [ "bash" ]

# vim: ft=dockerfile
