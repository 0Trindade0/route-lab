# Route Lab

Um laboratório de rotas em **C++17**, com experimentos reproduzíveis em **Python**. MVP de estudo voltado a estruturas de dados, testes e medição de desempenho.

## Demonstração

No grafo de exemplo, BFS encontra `A -> B -> D`, com **2 conexões**. Dijkstra encontra `A -> C -> E -> D`, com **custo 3**. As perguntas são diferentes: menor número de conexões versus menor soma dos pesos.

```text
A --10--> B --1--> D
|                 ^
1                 1
v                 |
C ------1-------> E        isolated
```

## Executar no Windows (PowerShell)

Requisitos: GCC com suporte a C++17 (`g++` no PATH) e Python 3.10+ para os testes de integração e experimentos. Não há pacotes Python externos.

```powershell
New-Item -ItemType Directory -Path build -Force
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -I include src/main.cpp -o build/route_lab.exe
./build/route_lab.exe examples/network.txt A D bfs
./build/route_lab.exe examples/network.txt A D dijkstra
```

Alternativa com CMake 3.16+ e um compilador configurado:

```text
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Geradores com múltiplas configurações, como Visual Studio, colocam executáveis em `build/Release`. Use esse caminho nos comandos seguintes e na variável `ROUTE_LAB_BINARY`.

## Formato de entrada e saída

- Grafo **direcionado**; declare dois arcos para uma conexão de mão dupla.
- `node ID` declara um vértice; IDs usam letras ASCII, números, `_` ou `-`.
- `edge ORIGEM DESTINO PESO` adiciona uma aresta entre nós já declarados.
- Pesos finitos e não negativos. Arestas paralelas, laços e peso zero são aceitos.
- Linhas vazias e comentários iniciados com `#` são ignorados.
- A CLI retorna JSON. `objective` é `hops` em BFS e `weight` em Dijkstra.
- Destino inacessível: `reachable: false`, `distance: null`, `path: []` e saída 0.
- Entrada inválida: mensagem no stderr e saída 2. `--help` mostra o uso.

## Testar

```powershell
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -I include tests/test_graph.cpp -o build/test_graph.exe
./build/test_graph.exe
python -m unittest discover -s tests -p 'test_*.py' -v
```

Os testes verificam rotas com objetivos diferentes, vértices isolados, origem igual ao destino, ciclos, pesos fracionários/zero, entrada inválida e o contrato da CLI. Para outro caminho de executável, defina `ROUTE_LAB_BINARY` antes de executar os testes Python.

## Experimentos

```powershell
python scripts/benchmark.py --binary build/route_lab.exe --sizes 50 200 1000 --repeats 5 --seed 42 --output reports/benchmark-local.csv
```

O gerador usa uma cadeia para garantir conectividade da origem ao destino e adiciona arestas aleatórias com semente fixa. Cada combinação recebe um aquecimento e várias medições.

**A métrica mede o processo inteiro**, incluindo inicialização, leitura, construção do grafo e serialização. Não isola o algoritmo. A geração dos dados fica fora do intervalo medido. Os resultados não demonstram que um algoritmo é universalmente mais rápido; ambos resolvem objetivos diferentes em grafos ponderados.

## Organização e decisões

- `include/graph.hpp`: grafo, parser, BFS e Dijkstra.
- `src/main.cpp`: interface de terminal e saída JSON.
- `tests/`: casos de comportamento em C++ e integração em Python.
- `scripts/benchmark.py`: geração e coleta de medições em CSV.
- `.github/workflows/ci.yml`: compilação, testes e experimento mínimo no GitHub Actions.

Listas de adjacência armazenam O(V + E) dados. BFS custa O(V + E), considerando acesso médio constante aos mapas. Dijkstra usa heap com entradas repetidas: O(V + E log(E + 1)) como limite simplificado, com até O(V + E) memória auxiliar. Hashes têm pior caso diferente do caso médio. Custos usam `double`; limites de precisão e overflow merecem atenção.

## Aprendizado e autoria

Primeira versão construída com assistência de IA para servir como base de estudo. A existência deste MVP não representa domínio prévio de todos os conceitos pelo autor. Próximas alterações devem ser pequenas, compreendidas e validadas por testes.

### Próximos exercícios

- [ ] Explicar a fila da BFS e a reconstrução do caminho sem consultar o código.
- [ ] Adicionar um teste antes de modificar uma funcionalidade.
- [ ] Separar a medição do algoritmo do tempo de leitura do arquivo.
- [ ] Comparar resultados com uma implementação independente em Python.
- [ ] Implementar exportação de uma rota para visualização.

Versão inicial: 0.1.0. Aplicação local de estudo; sem serviço web, interface gráfica ou dados de usuários.
