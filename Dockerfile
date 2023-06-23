# Dockerfile for Qt 6/Qt 5
# Автор: Denis

# Использование Ubuntu в качестве образа
FROM ubuntu

# Информация об авторе
LABEL maintainer="Denis <denrus2003@mail.ru>"

ENV TZ=Europe/Moscow
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone


RUN apt-get update
RUN apt-get install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools -y
RUN apt-get install build-essential -y


WORKDIR /root/server/
COPY main.cpp /root/server/
COPY mytcpserver.h /root/server/
COPY mytcpserver.cpp /root/server/
COPY gameServer.pro /root/server/

RUN qmake gameServer.pro 
RUN make

ENTRYPOINT ["./gameServer"]
