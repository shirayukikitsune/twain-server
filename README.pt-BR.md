# POC Scanner para Gliese

Este projeto é uma POC para o Gliese, que adiciona suporte para integração com scanner dentro de uma página web.adiciona

Para que haja a integração, é necessário que o navegador tenha acesso ao scanner do usuário, que não é possível sem um
agente integrador. Este agente cria um serviço em background no computador do usuário, que cria um servidor HTTP.

Este servidor HTTP expõe rotas que darão acesso ao scanner, como listar dispositivos, iniciar escaneamento, etc.

## Compilar

É necessário um compilador com suporte ao C++17. Compiladores testados:

- Microsoft Visual C++ 2017 (toolset 14.1)
- Microsoft Visual C++ 2019 (toolset 14.2)
- GNU C++ Compiler (g++) 8.3.0
- clang 8.0.0

### Pré-requisitos

Para compilar, além do compilador, é necessário ter o CMake instalado.
A versão mínima necessária é o 3.7.

### Bibliotecas

* [Boost 1.71](https://boost.org)
* [Loguru](https://github.com/emilk/loguru) (incluído como submódulo)
* [Nlohmann JSON](https://github.com/nlohmann/json) (incluído como submódulo)
* [Kitsune IOC](https://github.com/shirayukikitsune/ioc) (incluído como submódulo)

Para baixar as dependências incluídas:

* **Durante o `git clone`**: Utilizar o comando `git clone <repo> --recurse-submodules`
* **Após o `git clone`**: Utilizar o comando `git submodule update --init --recursive` a partir da raíz do projeto (o local onde está este arquivo)

### Preparação da compilação

O CMake restringe builds in-place (builds executando na raiz do projeto).
Portanto, será necessário criar um diretório para executar o build.
Nos *NIX, basta executar este comando, a partir da raiz do projeto: `mkdir build`
No Windows, pela linha de comando: `md build`

Feito isso, executar o CMake para gerar o cache:

```
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
```

#### Visual Studio

O Visual Studio 2017 ou mais recente suportam diretamente o CMake.
Basta abrir o diretório onde se encontra o CMakeLists.txt na IDE.

Mas também é possível gerar uma solução pelo CMake. Para isso:
* 32-bits: `cmake -DCMAKE_BUILD_TYPE=Release .. -G "Visual Studio 15 2017" -A Win32`
* 64-bits: `cmake -DCMAKE_BUILD_TYPE=Release .. -G "Visual Studio 15 2017" -A x64`

Ou então, pode utilizar o CMake-GUI.

### Compilando

Para compilar, basta entrar no diretório `build` e executar o seguinte comando:
`cmake --build . --target twain-server`  

Isso irá gerar o arquivo executável final.