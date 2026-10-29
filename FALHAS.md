# FALHAS

| data | o que quebrou | menor correção | prompt \| infra |
|---|---|---|---|
| 2026-10-29 | Revisor reprovou o guia 2× (6 e 7): sem desenho de peças com cotas, partida em pé antes da calibração, marcha lenta sem deslocar o corpo, instalador do arduino-cli fora do PATH | Lista de checagem de hardware antes de escrever: peças cotadas, onde entra cada fio físico, comportamento ao ligar | prompt |
| 2026-10-29 | Corpo da página apagado: `python … && rm` em linhas separadas, o `rm` rodou mesmo com o assert falhando (falso positivo do `{{` do código C++) | Encadear com `&&` na mesma linha; checar placeholder com regex `\{\{[A-Z_]+` | prompt |
| 2026-10-29 | Fases da marcha lenta erradas (sequência diagonal, menos estável) | Conferir a ordem de subida com script (fase ≥ duty) antes de desenhar o gráfico | prompt |
| 2026-10-29 | Tradução trocou caminho `firmware/montagem` por "assembly/montaje" e traduziu a saída literal do firmware | Pós-processar: grep `firmware/` e restaurar a linha de saída do Serial | prompt |
