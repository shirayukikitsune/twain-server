# POC Scanner para Gliese

Este projeto é uma POC para o Gliese, que adiciona suporte para integração com scanner dentro de uma página web.adiciona

Para que haja a integração, é necessário que o navegador tenha acesso ao scanner do usuário, que não é possível sem um
agente integrador. Este agente cria um serviço em background no computador do usuário, que cria um servidor HTTP.

Este servidor HTTP expõe rotas que darão acesso ao scanner, como listar dispositivos, iniciar escaneamento, etc.
