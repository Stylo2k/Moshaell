FROM ubuntu:20.04
# install without any user interaction
ENV DEBIAN_FRONTEND=noninteractive
RUN apt update && apt install make valgrind bison flex zip htop -y 

ENTRYPOINT [ "/bin/bash" ]