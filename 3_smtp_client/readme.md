Для получения исходного файла необходимо скомпелировать программу с следующими флагами

```
g++ smtp.cpp -o smtp -L/путь/до/openssl -lssl -lcrypto -ldl
```

Для того чтобы установить OpenSSL на Debian необходимо выполнить команду

```
sudo apt install openssl
```

Для того чтобы узнать местополодение OpenSSL в системе выполним команду

```
which openssl
```

Для запуска программы выполнит команду 

```
./smtp
```

