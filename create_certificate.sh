#!/bin/bash
echo "Create certificate"
openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem -days 100 -nodes -subj "/C=SE/L=Gothenburg/O=Example Company AB/CN=example.com"