FROM ubuntu:18.04 as osbuild

# node-gyp require make, gcc and python
RUN apt-get -y -m update && \
    apt-get install -y cmake g++ gfortran python3 curl
RUN curl -sL https://deb.nodesource.com/setup_10.x | bash
RUN apt-get install -y nodejs
RUN apt-get install -y build-essential

# RUN add-apt-repository ppa:jonathonf/gcc-7.1
# RUN apt-get update
# RUN apt-get install gcc-7 g++-7

WORKDIR /app
COPY package*.json ./
RUN npm install
COPY . .
CMD [ "npm", "run", "setup" ]
# COPY --from=osbuild /app/build/node-refprop-napi.node /Users/econ/basic-node-addon/docker-refprop.node
