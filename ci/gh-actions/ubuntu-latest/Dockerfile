# build for lgraph
FROM tugraph/tugraph-compile-centos7:latest

RUN mkdir -p /root/tugraph-db/
ENV WORKSPACE=/root/tugraph-db/
COPY ../.. /root/tugraph-db/
WORKDIR /root/tugraph-db/

RUN --mount=type=secret,id=CODECOV_TOKEN \
    export CODECOV_TOKEN=$(cat /run/secrets/CODECOV_TOKEN) && \
    mkdir build && cd build && cmake .. && make -j2 && cd output && ./fma_unit_test -t all
