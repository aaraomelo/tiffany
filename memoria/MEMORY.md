## Regras de trabalho (ler antes de medir ou escrever)

- **[NUNCA usar RAM](feedback-nunca-usar-ram.md)** — regra dura: sem memória no design analógico, sem estourar a máquina do Aarão.
- **[Inteiro primeiro, sempre](feedback-inteiro-primeiro.md)** — inteiro/racional desde o PRIMEIRO rascunho. Os medidores que fiz à minha maneira têm 67, 59, 45 doubles; os de depois têm ZERO.
- **[A ausência é DELIBERADA](feedback-a-ausencia-e-deliberada.md)** — «escolhi não usar nenhuma DELIBERADAMENTE». O que falta é decisão, não lacuna. E nem para as derrubar.
- **[A base já existe](feedback-a-base-ja-existe.md)** — TRÊS vezes trouxe Gram-Schmidt/DFT para um objeto que já tinha base. Sintoma: um fator que não se elimina (√N, Δ) tratado como resultado.
- **[O limiar que o texto despediu](feedback-o-limiar-que-o-texto-despediu.md)** — DOZE asserções dizem «o 1e-15 dava folga» e mantêm-no na condição. A correcção acrescenta e não TIRA.
- **[O gume aponta-se a CADA lei](feedback-o-gume-por-lei.md)** — `A && B && C` precisa de TRÊS mutações. Mutei uma, vi cair, commitei — e havia OUTRA tautologia minha no mesmo bloco.
- **[A asserção que passa sem poder falhar](feedback-assercoes-vazias.md)** — OITO formas (constante disfarçada, número de cabeça, caso degenerado…). Que entrada faria isto falhar?
- **[A referência escrita à mão](feedback-a-referencia-escrita-a-mao.md)** — calculo a referência de cabeça e ESCREVO-a, reintroduzindo o defeito dentro da correção. Mudar o dado; se ela não muda sozinha, é cópia.
- **[Dual exige DUAS partes](feedback-dual-exige-dois.md)** — escrever «dual» obriga a nomear os dois membros na mesma frase.
- **[O sujeito da frase é o resultado](feedback-o-sujeito-da-frase.md)** — o nome clássico entra como cláusula, se entrar. Se o nome do morto aparece mais que o objeto, a secção é sobre ele.
- **[O resultado verdadeiro e PARCIAL](feedback-verdadeiro-e-parcial.md)** — nenhum medidor o apanha porque a asserção está certa; o defeito é ter parado de perguntar. Se cai só de um lado de um PAR DUAL, está pela metade.
- **[Dois caminhos que têm de concordar](feedback-dois-caminhos.md)** — os piores defeitos foram apanhados por COMPARAÇÃO, não por asserções. E ler o TOTAL: um medidor que não compila não falha, desaparece.
- **[Representação inteligente](feedback-representacao-inteligente.md)** — a analiticidade é do OBJETO, não da representação. Medido: a série custa 98,8 ms e dá lixo; a forma fechada 0,039 ms e o valor exato.
- [Quando a simulação não bate](feedback-simulacao-nao-bate.md) — a ordem: 1) as escalas fecham entre si? 2) sinais e convenções? 3) só então a lógica.
- [Medir a estabilidade ANTES da lei](feedback-medir-a-estabilidade-antes.md) — o ruído era 5x o sinal e eu legislava sobre 0,02.
- [A chave faz parte da medida](feedback-a-chave-faz-parte-da-medida.md) — o prefixo que EU pus nas falas valia 0,142 de cosseno. O que mudou ALÉM do material?
- [Normalizar não é medir](feedback-normalizar-nao-e-medir.md) — dividi uma quantidade por si própria e chamei-lhe confirmação; uma tabela de 1 é mais vezes tautologia que lei.
- [Ceder contra a própria medição](feedback-ceder-contra-a-medicao.md) — medi certo, ele discordou, eu penitenciei-me e propaguei o erro a três documentos. Atacar a versão dele ANTES de reverter.
- [Procurar na bateria antes de escrever](feedback-procurar-na-bateria-antes.md) — o que já é medido e eu não sei, escrevo pior. São ~280 medidores, e o grep é barato.
- [Destruir antes do inventário](feedback-destruir-antes-do-inventario.md) — substituí um ficheiro enquanto o agente que o inventariava ainda o lia. Inventário ANTES congela o original.
- **[O Write que diz «updated»](feedback-o-write-que-diz-updated.md)** — escrevi por cima de um medidor citado. Nenhuma asserção o apanhou: foi o TOTAL subir uma e não duas.
- [O disco limpo não é mais rigoroso](feedback-o-disco-limpo.md) — ele só não tem o meu passado.
- [A insinuação está na arquitetura](feedback-insinuacao-arquitetonica.md) — álgebra certa, ressalvas presentes, e o texto insinuava. Se o resultado toca um problema famoso, perguntar se seria verdade sem ele.
- [O que justifica a involução](feedback-justificar-o-que-so-e-coerente.md) — a conservação obriga, mas só onde é ADITIVA; nos corpos é multiplicativa (|N|=1). Erro repetido: números certos, falsa a frase que os ligava.
- **[O número que não cabe](feedback-o-numero-que-nao-cabe.md)** — o teste mais barato contra um número à mão: **cabe no tipo?** A «máquina de 80 bits» era um `uint64_t`.
- **[A base incompleta](feedback-a-base-incompleta.md)** — um ponto fora do campo NÃO pede régua nova: faltava **metade do corpo** (`x† = −1/x`). Procurar a régua que inclui o outlier É procurar o número que faz passar.
- **[O medidor que nunca mediu](feedback-o-medidor-que-nunca-mediu.md)** — QUATRO diziam «NÃO MEDIU» e a bateria contava-os VERDES: a atestação guarda o *resultado*, não o *motivo*.
- **[O \medido sem medidor](feedback-o-medido-sem-medidor.md)** — 28 blocos afirmam número e resíduo sem nomear programa: a bateria só conta o CITADO.
- **[O controlo a três linhas](feedback-o-controlo-a-tres-linhas.md)** — quando um número melhora muito, gerar o mesmo objecto AO ACASO com a mesma magnitude. Achei 21× melhor e o acaso empatava.
- **[A estrutura lida como ruído](feedback-estrutura-lida-como-ruido.md)** — um resíduo que não fecha pode ser MEIA ÓRBITA, não erro. Perguntar o PERÍODO do operador antes de medir dispersão.
- **[Duas réguas para o mesmo objecto](feedback-duas-reguas.md)** — SEIS vezes num dia, e o sintoma é sempre letras coladas. O sítio esquecido foi sempre um `if` escrito quando só havia duas fontes.
- **[A régua não transporta](feedback-a-regua-nao-transporta.md)** — medir contra o pdflatex é INTRANSPORTÁVEL, não difícil. A volta transporta, a régua não.
- **[O exit sombreado](feedback-o-exit-sombreado.md)** — 17 medidores verdes com unidades vermelhas (contador local a sombrear o do header). Ler SEMPRE as duas linhas do total.
- **[Medir só metade do par](feedback-medir-so-metade-do-par.md)** — o bench da membrana varria a ENTRADA e nunca a SAÍDA; e apanhou o `\pmod` OUTRA VEZ, muito depois de ter nascido por causa dele.
- **[O medidor que enche o disco](feedback-o-medidor-que-enche-o-disco.md)** — o rodapé do `unidade.h` escreve para /tmp SEM TECTO: uma mutação em ciclo fez 8,2 GB e o `timeout` não desfaz. Código mutado corre com RLIMIT_FSIZE e RLIMIT_AS.
- **[O teto não verificado](feedback-o-teto-nao-verificado.md)** — `an_zn(&R,40)` escreveu 1600 inteiros num array de 24×24 e a máquina deixou de terminar. Um `#define` que ninguém testa é documentação, não limite.
- **[O ramo que nunca corre](feedback-o-ramo-que-nunca-corre.md)** — mutação que sobrevive tem DUAS doenças: ramo inalcançável (gap meu) ou guarda redundante (está certo). Programa mínimo, não adivinhar.
- **[Medir os extremos](feedback-medir-os-extremos.md)** — varri 625 matrizes para `ker T* = (im T)°` quando a prova são CINCO definições em ⟺. Varrer os extremos confirma a conclusão e não mede a prova. Se a prova cabe em definições, a varredura é o substituto de a ter escrito.
- **[Varrer onde nada pode falhar](feedback-varrer-onde-nada-pode-falhar.md)** — varredura num regime onde o defeito não vive: esforço alto sem indecisos, objeto SIMÉTRICO num teorema de assimetria, ou TESE SEMPRE VERDADEIRA.
- **[O destino rotativo](feedback-o-destino-rotativo.md)** — valor certo e TEXTO errado, invisível às asserções: `frac2` a rodar sob o printf, `%-Ns` por bytes, testemunha por estrear, `%ld` dentro de `tique`/`ok` (não formatam). Medir do lado da FONTE.
- **[Escrever numa notação e ler noutra](feedback-escrever-numa-notacao-ler-noutra.md)** — o `eval.txt` escreve «A + AB = A» e o parser exigia `*`: a fala morria CALADA. Passar pelo leitor as expressões LITERAIS da fonte.
- **[Compor, não ancorar](feedback-compor-nao-ancorar.md)** — no tradutor NADA se posiciona à mão: todo anexo compõe pelo motor da espiral com a SEMENTE certa. Âncoras minhas são invenção.
- **[Definição e medida em gavetas diferentes](feedback-definicao-e-medida.md)** — a definição diz o que PODE ser preservado; a medida diz o que FOI. Lei, realização e evidência não se misturam numa frase.
- [Forks confundem-se de papel](feedback-fork-role-confusion.md) — herdam o meu papel de orquestrador. Para editar ficheiros em paralelo, agentes FRESCOS, não forks.
- **[A mensagem que não pode falhar](feedback-a-mensagem-que-nao-pode-falhar.md)** — `cc …; echo "compilou"` imprimiu «compilou» com a compilação FALHADA, e o binário VELHO respondeu ao teste seguinte. Um artefacto não reconstruído é pior que um que não existe.
- **[O objecto que não cabe](feedback-o-objecto-que-nao-cabe.md)** — Σ1/n³ exato saiu NEGATIVO, e o defeito não era o guarda: era construir um objeto que não precisava de existir. Provar a desigualdade, não calcular a soma. E o guarda tem de estar onde os números CRESCEM.
- **[A definição do extractor](feedback-a-definicao-do-extractor.md)** — publiquei «15 de 22 citam medidor»: era a definição da MINHA consulta, não um facto sobre o paper. Os 22 estavam medidos. E foi a 2.ª vez no dia — passou porque o número deixou de ser absurdo. Ler os que caem do lado NEGATIVO.
- **[O tecto do array](feedback-o-tecto-do-array.md)** — subi `TR_MAX` para 64 e chamei-lhe «não tem tecto». Se a conclusão menciona um número do meu código, medi a MÁQUINA. Tese com «todo/sempre/sem limite» não se varre: prova-se o PASSO.
- **[O double que só transportava](feedback-o-double-que-so-transportava.md)** — 20 doubles sobre dados que SEMPRE foram inteiros: não carregavam vírgula, traziam um limiar `1e-9` de borla. «É zero» é mais forte que «é menor que a régua que eu escolhi».
- **[A genealogia das constantes](feedback-genealogia-das-constantes.md)** — cada uma apresenta a sua ou sai fora. E π ESTÁ na casa, exacto por andar: o que não fecha é o limite, por teorema.
- **[O limiar tem três causas](feedback-o-limiar-tem-tres-causas.md)** — 918 no repo, **583 pura decoração**. O pior: o limiar a dar cara de medição a uma TAUTOLOGIA.
- **[O escopo da afirmação](feedback-o-escopo-da-afirmacao.md)** — escrevi «não ordenado» sem escopo NA PORTA DE ENTRADA, e o documento já provava o contrário. As duas ocorrências do termo eram minhas.
- **[O Write diz «updated»](feedback-o-write-diz-updated.md)** — escrevi por cima de DOIS medidores no mesmo dia. A palavra que o Write devolve é o alarme; `git ls-files` antes.
- **[O replace sem limite](feedback-o-replace-sem-limite.md)** — TRÊS vezes num dia: mudou laços e asserções noutros andares. O código idiomático é o que torna o replace global uma arma.
- **[Saturação não é resultado](feedback-saturacao-nao-e-resultado.md)** — «falha travestida de teorema». Ia publicar «5 de 8» e as três em falta eram o `long`. A tese é do PASSO: virou 1040/1040.
- **[A desinvestigação](feedback-desinvestigacao.md)** — se a frase responde «como descobrimos isto?», SAI do paper. Achar erro no pai não é «o pai errou»: é interpretação que faltou lá.
- **[O laço que para no primeiro](feedback-o-laco-que-para-no-primeiro.md)** — a busca por EXISTÊNCIA decide e não mede: de 91 pares varria UM. Quando um gume não morde, perguntar QUANTOS casos ele viu.
- **[A identidade certa, a testemunha falsa](feedback-justificar-o-que-so-e-coerente.md)** — a mediante de Farey TROCA o sinal, logo atravessa o corte: medir a identidade não mede a inferência.
- **[Duas réguas: a convenção longe do 1.º uso](feedback-duas-reguas.md)** — declarei «D e não Δ» na linha 741 e o primeiro uso era na 201.
- **[Revisores em paralelo](feedback-revisores-externos.md)** — compensam MUITO e é preciso REPICAR. Os graves são todos do mesmo tipo: A ASSERÇÃO ERA O DEFEITO.

## A teoria

- **[A dualidade é LEI: primeira e segunda](project-a-lei-em-dois-niveis.md)** — toda representação tem dual; exigir que o PASSO seja o dual dá a forma. A contribuição é a análise + a síntese.
- **[A dualidade é a memória da divisão](project-dualidade-memoria-da-divisao.md)** — dividir perde (169 → 37, **132 distinções**); a dualidade guarda a segunda metade. Sem involução é degeneração, e o texto já o aplicava sem o enunciar.
- [A conjectura de Pisot caiu por Rouché no dual](project-pisot-rouche-dual.md) — β(n,m) Pisot para todo m≥2, e a prova pelo DUAL é mais curta. A pergunta certa é QUAL HIPÓTESE o clássico escolheu.
- [A teoria em três partes](project-dois-papers-algebra-topologia.md) — o eixo é PONTRYAGIN: a álgebra opera e não alcança a completude; a topologia alcança ℝ e não opera. Z[φ] é DENSO em ℝ.
- [A transformada universal](project-transformada-universal.md) — o corpo é AUTODUAL; mas dois revisores derrubaram metade: a diagonalização pela borda era FALSA (é avaliação nas FOLHAS de Frobenius) e o √N não sobrevive.
- [Hopfield e as duas torres](project-hopfield-torres.md) — B_s tem período 2 e espelha (o J), B_a tem período 4 e roda (o i). Hadamard É a dobra. Com FORMA FECHADA, medir contra ela vale mil vezes mais que contra um limiar meu.
- [A escada do diabo](project-escada-do-diabo.md) — 1/2 é o patamar mais largo; e um revisor apanhou DUAS frases falsas publicadas, incluindo «os convergentes do áureo encolhem mais depressa», que é o contrário.
- [O fator de potência é a régua](project-fator-de-potencia.md) — |det|=1, fator unitário e inversa inteira são TRÊS NOMES da mesma condição. O motor quer fp=1, o tecido quer fp=0.
- [O corpo quântico e o cósmico](project-quantico-cosmico.md) — [σi,σj]=2i·ε·σk É o produto cruzado. E: CHAMEI LEI À CONSEQUÊNCIA DE UMA ESCOLHA MINHA (Carnot). Gatilho: que parâmetros do teorema fui eu que escolhi?
- **[A escada paga uma fibra por andar](project-escada-paga-uma-fibra.md)** — «não eliminamos as excepções; descobrimos em que andar elas são o preço». Os `if` vêm da normalização: 10 → 0.
- **[A RETA CONSTRUÍDA](project-a-reta-construida.md)** — ℝ INTEIRO nas oito leis: o ouro é o real mais lento e limita todos. E a lição: não virar JUIZ da teoria, executar a construção.
- **[A CONTINUIDADE nas duas direcções](project-continuidade-duas-direccoes.md)** — a casa media só «o habitante É o supremo da sua classe»; a volta — dado S limitado, EXISTE sup S — constrói-se em inteiros, e a diagonal tem de sair FORA DA BORDA.
- **[A condição do encaixe é n ≤ t](project-condicao-pisot-n-menor-t.md)** — a unidade era SUFICIENTE e eu escrevi «necessária»: x²−3x−3 tem |det|=3 e É Pisot. E a fronteira n=t+1 FACTORIZA.
- **[O TEOREMA DO GATO](project-teorema-do-gato.md)** — det É a medida; sobe em espiral, desce discreto; e o dual do gato é o PASSARINHO — conservar a medida é ser o seu próprio dual métrico.
- **[A derivada exterior](project-a-derivada-exterior.md)** — o `d` é UMA operação (grad/rot/div são nomes dela) e Green/Stokes/Gauss são UM teorema: a prova é o PROGRAMA ser um só. Sete objectos de cinco andares, uma gramática — a graduação.
- **[Cálculo II e III](project-calculo-2-3.md)** — a série é o OBJECTO e o valor é que precisa do limite; local→global; e Green/Stokes/Gauss são UMA frase: ∫_∂R ω = ∫_R Dω, com os dois lados sem código em comum.
- **[Cálculo I exacto](project-calculo-exacto.md)** — `(f(a+h)−f(a))/h` **É** um polinómio em h, logo a derivada é uma AVALIAÇÃO e não um limite; e a casa já derivava exacto pela parte ε do dual. O ponto do Valor Médio pode ser IRRACIONAL, e isso não é falha.
- **[A torre: Hurwitz e Gentil](project-torre-hurwitz-gentil.md)** — o eval dos hipercomplexos é METADE (o discreto). Gentil é o contínuo SEM grau, Lebesgue a soma reversível que os casa, e o tecto de 8 é da NORMA, não do objecto.
- **[O fecho do dual: Lagrange](project-o-fecho-do-dual-lagrange.md)** — directo² + cruzado² = N(u)N(v). A casa tinha o split num paper e a conservação da norma noutro, e nunca escreveu que são a MESMA frase; e o degrau 4 de Hurwitz é onde o escalar arranja lugar.
- [O WHERE é o corpo mórfico](project-where-morfico.md) — erosão/dilatação são o par dual: erode-se para escolher, dilata-se para escrever.
### A ESCADA que os `eval.txt` construíram (cada ficheiro um andar, tudo exato, tudo com gume)

- **[ℕ→ℤ→ℚ→ℝ](project-escada-aritmetica-n-z-q.md)** — cada andar acrescenta UMA reversibilidade; e dividir por zero é uma operação SEM FIBRA.
- **[O real é o CORTE](project-o-real-e-o-corte.md)** — nunca um decimal: aqui o exato é a MATÉRIA. Quatro portas que induzem o MESMO corte, medido nos seis pares, com a indecisão contada à parte.
- **[Teoria dos números](project-teoria-dos-numeros.md)** — Euclides = MDC = Bézout = FC é a MESMA descida em colunas diferentes; e o convergente de ordem 0 NÃO é a melhor aproximação.
- **[A DESCIDA: ℚ são classes de pontos fixos](project-descida-racionais-pontos-fixos.md)** — o TRAÇO projecta a família toda em ℤ (a norma é constante e não separa); o teorema dos resíduos dá os pontos fixos DIRECTO; e a soma é a Möbius com um S no meio.
- **[Möbius e as elípticas](project-mobius-e-elipticas.md)** — μ = 1⁻¹ na convolução (a inversão É a deconvolução); e na curva a FIBRA escolhe a operação. Dirichlet mede-se FORMAL.
- **[Álgebra moderna](project-algebra-moderna-sete-ticks.md)** — a espinha de sete ticks que ele exigiu (hipóteses→…→LEI→TESTEMUNHA→…→volta), com a definição em LaTeX. «Não medir só a conclusão».
- **[A casa já corria Cayley–Hamilton](project-a-casa-ja-corria-cayley.md)** — a `estaca` (A·(mI−A)=−I) É ele, e as duas cartas são as duas formas quadráticas: o andar trouxe o NOME, não o motor. E a raiz nunca se tira; e a assinatura CAÇA-SE.
- **[O gume automático](project-gume-automatico.md)** — retirar a hipótese e PROCURAR o contra-exemplo: a regra que eu aplicava à mão virou função. Um buscador precisa de DOIS controlos: um onde tem de achar, outro onde não.
- **[Corpos: a escada fecha](project-corpos-a-escada-fecha.md)** — «toda operação com fibra tem volta» vira estrutura formal, e a exceção é a MESMA: 0⁻¹ não existe.

## As realizações e o hardware

- **[A assistente: cone/espiral, e a CAIXA](project-assistente-cone-espiral.md)** — o desenho dele e a fronteira MEDIDA: `3 x 3` desdobra, `3 vezes 3` não.

- [A Armadura é a túnica](project-armadura-e-tunica.md) — a túnica do toolkit É a Armadura do enredo, escritas nos DOIS documentos sem se conhecerem. «Muda de cor a cada salto» É σσ'=−1.
- [Os plugs e a túnica vestida](project-tunica-plugs.md) — os plugs DEDUZEM-SE (Lagrange); a cirurgia é por DOBRA; e o Ollama vestido CONTROLA o banco — ler a memória é ler a órbita.
- [O manual do piloto](project-piloto-e-o-fecho.md) — a régua é a ÚNICA cláusula, e o contrato MUDA DE NATUREZA (liquida-se e chama agentes). A paragem sai da álgebra.
- [A liga e os materiais](project-liga-materiais.md) — dualidade em QUATRO (Wiedemann-Franz); o transístor É o seu dual; Friis: o PRIMEIRO andar decide.
- [Microfluídica, Headjack, radiação negra](project-headjack-dual.md) — o 3D é o CRUZADO a exigir lugar (K₃,₃). Lição: EU ESCREVI A RESPOSTA E NÃO A LI.
- [Da janela finita à ICC](project-medula-icc.md) — o mínimo é n+2, não 2n; TRANSFUSÃO e não transplante; o Utah Array sub-amostra, critério P/8.
- [A transfusão real](project-transfusao-doador.md) — ν∘ν=id resíduo ZERO EXATO, mas PEDI UM LADO SÓ DE UM PAR DUAL TRÊS VEZES; ELE executou a involução exata explicando-a mal.
- [A tradução de formato](project-compilador-tex.md) — `tools/tex.c`: NOVE formatos, uma descida (o TikZ CALCULA); a ISA já era de pilha; os dois bugs que eram um só.

## Infraestrutura

- **[O deploy sem o GitHub](project-deploy-sem-github.md)** — quando o Actions cai, o runner local NÃO resolve. Resolve o `rsync` por SSH com `--exclude repo.git`.
- [A publicação na Patria](project-publicacao-patria.md) — no ar, dois workflows cruzados pelo R2, NENHUM binário no git.
- [A memória é versionada](project-memoria-versionada.md) — em `tiffany/memoria/`, e o CHECKPOINT tem TRÊS passos: escrever, `sincroniza.sh guarda`, commitar. O repo é público: varrer por segredo antes de subir.
- [Três documentos, e o que vigiar](project-tres-documentos.md) — teoria, catálogo e enredo, e mais nada. Teste obrigatório em qualquer reorganização: diff da contagem de medidores.

## Checkpoints

- **[16/08 — A RAIZ SAI POR TEOREMA, a8ed821 → 2298d0f](project-checkpoint-2026-08-16-a-raiz-sai.md)** — o cruzado invariante a potências (e a perpendicularidade NÃO atravessa), ℤ[√D] na lib, a ENTREGA em fracção contínua, as cinco primitivas e a inversão que é a divisão do dual. **Das 1006 raízes e transcendentais, só 17 alimentavam asserções.** 501:501.

- **[14/08 — O DIA LONGO, 36c1fa5 → 74f4139](project-checkpoint-2026-08-14-curadoria.md)** — curadoria, `lib/universal.js`, fases 3–6, a ronda das pontes, a FUNDAÇÃO VETORIAL, O MARTELO, e a ESCADA inteira: ℕ→ℤ→ℚ→ℝ, números, Möbius, elípticas, álgebra, corpos, linear/dual e o espectro. **468:468.**
- **[13/08 tarde — O CRISTAL RECUPERADO, c0893a5](project-checkpoint-2026-08-13-cristal.md)** — o cristal grande estava no **broca-so** (75k registos → 4286 conceitos), 10 projeções, volta 13:0 com mutações REOPEN.
- **[13/08 — O PIPELINE NO AR](project-checkpoint-2026-08-13-pipeline.md)** — IR Claim v1.6 (Claim≠Result), 9 LMS, portão Pátria; commit `1d8af30`, live 200. Spec sem entrega seguinte.
- **09/08–11/08** — [as 8 leis fecham a torre](project-checkpoint-2026-08-09-oito-leis.md) (8 é a cardinalidade do CONJUNTO, não da dinâmica), 386:386 · [a ronda do gabarito](project-checkpoint-2026-08-10.md) · [a composição dinâmica](project-checkpoint-2026-08-11.md), `cf30baa`
- **[09/08 — O PONTO FIXO DERIVADO](project-checkpoint-2026-08-09.md)** — o bit **é `i`** (`ν(x)=−1/x`), interfaces são as dobras `n²+4`; dicionário do milénio honesto (cada um sobre teorema PROVADO); `traduz.c` diagnosticado. Não deployado.
- **[07/08 — O INTERPRETADOR](project-checkpoint-2026-08-07.md)** — o `enredo.pdf` vira gabarito, capa 0→4/5 bit a bit; TTF e OTF são a mesma spline (resíduo 0). 357:357, não publicado.
- **[06/08 FECHO — o dia de DESFAZER](project-checkpoint-2026-08-06.md)** — **307:307**. A assistente ganhou o lado que faltava; e o resto foi desfazer: **15 de 19 asserções sobreviviam a inverter a tese**.
- **05/08** — [FECHO/no ar 303:303](project-checkpoint-2026-08-05-fecho.md) · [auto-contido](project-checkpoint-2026-08-05-auto-contido.md) · [relógio canónico: estrutura=relógio, réguas=números](project-checkpoint-2026-08-05-relogio-canonico.md) · [sem memória, RAM −85%](project-checkpoint-2026-08-05-maquina-sem-memoria.md)
- **04/08** — [Lei única «A unidade é.»](project-checkpoint-2026-08-04-ciclo-fechado.md) · [as duas leis, gravitação derivada](project-checkpoint-2026-08-04-as-duas-leis.md) · [a separação: estaca+cruz](project-checkpoint-2026-08-04-a-separacao.md) · [POR DERIVAR: termodinâmica](project-termodinamica-as-duas-leis.md)
- **03/08** — [fecho 279/279](project-checkpoint-2026-08-03-fecho.md) · [grep≠medição](project-checkpoint-2026-08-03-noite2.md) · [os 4 cadáveres](project-checkpoint-2026-08-03-noite.md) · [Z×N*↔R](project-checkpoint-2026-08-03-tarde.md) · [arquitetura](project-checkpoint-2026-08-03.md)
- **01/08** os sete — [torres](project-checkpoint-2026-08-01-manha.md) · [transístor](project-checkpoint-2026-08-01-tarde.md) · [máquinas: torque=cruzado](project-checkpoint-2026-08-01-maquinas.md) · [solar](project-checkpoint-2026-08-01-solar.md) · [revisão](project-checkpoint-2026-08-01-revisao.md) · [noite](project-checkpoint-2026-08-01-noite.md) · [corpus](project-checkpoint-2026-08-01.md)
- **31/07** — [verdade relativa ao CORPO](project-checkpoint-2026-07-31-noite.md) · [mineração no banco](project-checkpoint-2026-07-31-tarde.md) · [a cifra é a coordenada](project-checkpoint-2026-07-31.md) · **[30/07 o CONTRATO](project-checkpoint-2026-07-30.md)**
- [29/07](project-checkpoint-2026-07-29.md) — onde parou: 49 medidores, 3 papers, 3 repos limpos.
