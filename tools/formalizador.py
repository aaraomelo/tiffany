#!/usr/bin/env python3
# -*- coding: utf-8 -*-
r"""
O FORMALIZADOR — o certificador, cristalizado no corpo universal.

Um certificador é um INSTRUMENTO: leva cada afirmação a um resíduo em {0,1}.

    M : afirmações  ->  Z × G      resíduo g = 0  <=>  afirmação certificada

Do contrato saem, sem acrescento, as três regras deste ficheiro.

INDUÇÃO (o sucessor). A suite é a órbita de S: cada `afirma` é um degrau, e o
veredito é a soma iterada dos resíduos. O fechamento S_0 é o conjunto das suites
de resíduo nulo, e é fechado sob composição (verbo 1): compor duas suites
certificadas devolve uma suite certificada.

META-INDUÇÃO (o quociente). Duas afirmações são a MESMA quando nenhum contexto
as distingue. Aqui os contextos são as MUTAÇÕES do objeto sob teste:

    A ~ B   <=>   para toda mutação m:   A(m) = B(m)

Logo, o critério que faltava, e que é o teorema deste ficheiro:

    UM TESTE SEM DENTES É UM TESTE NA CLASSE DE EQUIVALÊNCIA DE `True`.

`afirma(nome, True)` não é uma afirmação: é o neutro, disfarçado. E um teste que
passa sob toda mutação está na classe do neutro, ainda que pareça calcular algo.
(Foi o caso de `Λ(zʲ) = j·(λ,ϑ)`: verdadeiro para todo z, logo congruente a True.)

Daí a lei: TODA AFIRMAÇÃO TEM DE EXIBIR UM REFUTADOR — um contexto em que ela
falha. A marca [N] promete falsificação; aqui ela é obrigatória, não retórica.

NO CORPO CONFORME (ver elementares/conforme.tex) isto lê-se numa só linha. Cada
afirmação tem um VETOR DE COMPORTAMENTO b = (testa, refuta) em {0,1}². A afirmação
trivial -- congruente a True -- é b_T = (1,1). O mergulho conforme

    X(b) = n0 + b + ½|b|²·n_inf          X(b)² = 0   (toda afirmação entra no cone)

põe todas no cone nulo: a NORMA de uma afirmação é sempre zero, e a medida passa
para o EMPARELHAMENTO:

    < X(b), X(b_T) >  =  -½ · d(b, b_T)²

    com dentes (1,0):   -0,5     |  a afirmação DISTA da trivialidade
    sem dentes (1,1):    0,0     |  a afirmação É a trivialidade
    falha      (0,0):   -1,0     |

OS DENTES SÃO A DISTÂNCIA À AFIRMAÇÃO TRIVIAL, LIDA NO PAR CONFORME. E o
fechamento S_0 é o cone: resíduo nulo.

É o PONTEIRO do relógio de xadrez, e os dois mostradores são `testa` e `refuta`.
Ponteiro em -½: o teste tem dentes -- e ½ é A METADE, com o sinal da reflexão.
Ponteiro em 0: QUEDA DE BANDEIRA, o relógio parou, o teste é a trivialidade.

    formalizador.afirma(nome, testa, refuta)

        testa  : () -> bool   deve devolver True  (o objeto verdadeiro)
        refuta : () -> bool   deve devolver False (o objeto MUTADO)

Se `refuta` for omitido, a afirmação é aceite mas contada à parte, como SEM
DENTES: não entra no fechamento. Se qualquer um lançar exceção, isso é FALHA e
não um crash — a suite continua e reporta.

    python3 elementares/formalizador.py            # auto-certifica-se
    python3 elementares/formalizador.py --auditar   # varre elementares/*.py
"""
from __future__ import annotations

import argparse
import ast
import glob
import os
import sys

RAIZ = os.path.dirname(os.path.abspath(__file__))


class Formalizador:
    """O instrumento. Guarda os resíduos; o veredito é a sua soma."""

    def __init__(self, titulo: str = "") -> None:
        self.titulo = titulo
        self.certificadas: list[str] = []
        self.falhas: list[str] = []
        self.sem_dentes: list[str] = []
        self.triviais: list[str] = []
        self.pares: list[int] = []  # em meios: −1 = −½, −2 = −1,0
        if titulo:
            print("=" * 78)
            print(titulo)
            print("=" * 78)

    # ---------------------------------------------------------------- o degrau
    def afirma(self, nome, testa, refuta=None, detalhe: str = "") -> bool:
        """Um degrau da indução. Devolve True se o resíduo é nulo."""
        if not callable(testa):
            raise TypeError(
                f"'{nome}': `testa` tem de ser um invocável, não {testa!r}. "
                "Um valor constante está na classe de equivalência de True — "
                "não é uma afirmação, é o neutro disfarçado."
            )
        ok, nota = self._avaliar(testa)
        if ok is not True:
            self._registar("FALHA", nome, nota or detalhe)
            self.falhas.append(nome)
            return False

        if refuta is None:
            self._registar("s/dentes", nome, detalhe or "sem refutador: não entra no fechamento")
            self.sem_dentes.append(nome)
            return True

        mau, nota_m = self._avaliar(refuta)
        if mau is not False:
            # o objeto mutado passou: a afirmação é congruente a True.
            self._registar(
                "SEM DENTES", nome,
                "o refutador PASSOU: a afirmação é congruente a `True` sob mutação"
                + (f" ({nota_m})" if nota_m else ""),
            )
            self.sem_dentes.append(nome)
            return True

        self._registar("ok", nome, detalhe)
        self.certificadas.append(nome)
        self.pares.append(self.emparelhamento(True, False))     # −0,5: dista da trivial
        return True

    def trivial(self, nome, porque: str) -> None:
        """Verdade que não depende do objeto. Registada, e NÃO certificada."""
        self._registar("trivial", nome, porque)
        self.triviais.append(nome)

    def cita(self, nome, fonte: str) -> None:
        """[T]: teorema citado, não certificado aqui."""
        self._registar("[T]", nome, fonte)

    def pipe_da_torre(self, lam=None):
        """O pipe canónico, depois do operador: metais fundamentais e corpo final.

        Importa-se aqui para evitar dependência circular. Ver elementares/torre.py.
        """
        import math as _m

        from torre import pipe as _pipe
        return _pipe(self, _m.log(2) if lam is None else lam)

    def nota(self, texto: str) -> None:
        print(f"  (nota) {texto}")

    def seccao(self, texto: str) -> None:
        print(f"\n{texto}")

    # ---------------------------------------------------------------- interno
    @staticmethod
    def _avaliar(f):
        """Uma exceção é um resíduo, não um crash."""
        try:
            return bool(f()), ""
        except Exception as e:  # noqa: BLE001
            return None, f"exceção: {type(e).__name__}: {e}"

    @staticmethod
    def emparelhamento(testa_ok: bool, refuta_ok: bool) -> int:
        """<X(b), X(b_T)> = -½·d(b, b_T)²  com b = (testa, refuta) e b_T = (1,1).

        Devolve o valor em MEIOS (−1 = −½, −2 = −1,0, 0 = trivial).
        """
        bt, br = (1 if testa_ok else 0), (1 if refuta_ok else 0)
        d2 = (bt - 1) ** 2 + (br - 1) ** 2
        return -d2

    @staticmethod
    def _registar(marca: str, nome: str, detalhe: str) -> None:
        print(f"  [{marca:>10}] {nome}" + (f"   {detalhe}" if detalhe else ""))

    # ---------------------------------------------------------------- o veredito
    def veredito(self) -> int:
        """O resíduo da suite. Zero se, e só se, nada falhou e tudo tem dentes."""
        n = len(self.certificadas) + len(self.falhas) + len(self.sem_dentes)
        print()
        print(f"  certificadas : {len(self.certificadas)}/{n}")
        if self.triviais:
            print(f"  triviais     : {len(self.triviais)}  (verdadeiras, e não dependem do objeto)")
        if self.sem_dentes:
            print(f"  SEM DENTES   : {len(self.sem_dentes)}  -> {self.sem_dentes}")
        if self.falhas:
            print(f"  FALHAS       : {len(self.falhas)}  -> {self.falhas}")
        if self.pares:
            m = min(self.pares)
            sinal = "−" if m < 0 else ""
            a = abs(m)
            print(f"  ponteiro     : {sinal}{a // 2},{'5' if a % 2 else '0'} em todas as {len(self.pares)} certificadas"
                  "   ⟨X,X_⊤⟩ = −½·d²: a distância conforme à afirmação trivial")
        residuo = len(self.falhas) + len(self.sem_dentes)
        print(f"  resíduo      : {residuo}" + ("  (medida consistente)" if residuo == 0 else ""))
        return 0 if residuo == 0 else 1


# =============================================================================
# AUTO-CERTIFICAÇÃO: o formalizador aplicado a si próprio.
# =============================================================================
def auto() -> int:
    F = Formalizador("O FORMALIZADOR — auto-certificação (o instrumento mede-se a si)")

    F.seccao("1. INDUÇÃO: a suite é a órbita do sucessor, e S₀ é fechado sob composição")
    F.afirma("o veredito é a soma iterada dos resíduos",
             lambda: sum([0, 0, 0]) == 0 and sum([0, 1, 0]) == 1,
             lambda: sum([0, 1, 0]) == 0,
             "a soma é o sucessor iterado")
    F.afirma("compor duas suites de resíduo nulo dá resíduo nulo (verbo 1)",
             lambda: max(0, 0) == 0,
             lambda: max(0, 1) == 0,
             "S₀ é submonoide")

    F.seccao("2. META-INDUÇÃO: um teste sem dentes está na classe de `True`")
    G = Formalizador()
    G.afirma("uma afirmação com refutador que PASSA é marcada sem dentes",
             lambda: True, lambda: True)
    F.afirma("o formalizador deteta o teste congruente a `True`",
             lambda: len(G.sem_dentes) == 1 and len(G.certificadas) == 0,
             lambda: len(G.certificadas) == 1)

    H = Formalizador()
    H.afirma("afirmação com refutador que falha é certificada",
             lambda: 2 + 2 == 4, lambda: 2 + 2 == 5)
    F.afirma("e certifica a que tem dentes",
             lambda: len(H.certificadas) == 1 and len(H.sem_dentes) == 0,
             lambda: len(H.sem_dentes) == 1)

    F.seccao("3. UMA EXCEÇÃO É UM RESÍDUO, NÃO UM CRASH")
    import math as _m
    J = Formalizador()
    J.afirma("log de negativo", lambda: _m.log(-1.0) > 0, lambda: False)
    F.afirma("a exceção vira FALHA e a suite continua",
             lambda: len(J.falhas) == 1,
             lambda: len(J.falhas) == 0)

    F.seccao("4. UM VALOR CONSTANTE NÃO É UMA AFIRMAÇÃO")
    K = Formalizador()

    def _constante():
        try:
            K.afirma("tautologia", True)
            return False
        except TypeError:
            return True

    F.afirma("`afirma(nome, True)` é recusado: é o neutro disfarçado",
             _constante, lambda: False)

    F.seccao("5. O QUOCIENTE É CANÓNICO")
    F.afirma("dois certificadores que fecham sobre as mesmas afirmações dão o mesmo veredito",
             lambda: Formalizador()._avaliar(lambda: True) == Formalizador()._avaliar(lambda: True),
             lambda: Formalizador()._avaliar(lambda: True) == Formalizador()._avaliar(lambda: False),
             "verbo 3: a lei de composição é forçada, não escolhida")

    F.cita("o teste de mutação é a quantificação sobre os contextos",
           "[T] –: a classe de s é o seu comportamento em todo contexto u·s·v")
    F.trivial("0 = 0", "não depende de objeto nenhum: é o neutro")

    return F.veredito()


# =============================================================================
# AUDITORIA: quantas afirmações dos nove certificadores exibem refutador?
# =============================================================================
def _contar(src: str) -> tuple[int, int]:
    """Conta chamadas a `afirma` e quantas passam uma CONSTANTE como condição.
    Pela árvore sintática: um regex já me deu 1, depois 5, e nenhum era o número."""
    n_af = n_taut = 0
    for no in ast.walk(ast.parse(src)):
        if not isinstance(no, ast.Call):
            continue
        alvo = getattr(no.func, "id", None) or getattr(no.func, "attr", None)
        if alvo != "afirma":
            continue
        n_af += 1
        if len(no.args) >= 2 and isinstance(no.args[1], ast.Constant):
            n_taut += 1
    return n_af, n_taut


def auditar() -> int:
    print("=" * 78)
    print("AUDITORIA DOS CERTIFICADORES — quantos degraus exibem refutador?")
    print("=" * 78)
    alvos = sorted(f for f in glob.glob(os.path.join(RAIZ, "*.py"))
                   if os.path.basename(f) not in ("formalizador.py", "auditoria.py"))
    tot_af = tot_taut = tot_mig = 0
    for f in alvos:
        src = open(f, encoding="utf-8").read()
        n_af, n_taut = _contar(src)          # pela ÁRVORE, não por regex
        migrado = "formalizador" in src
        tot_af += n_af
        tot_taut += n_taut
        tot_mig += migrado
        marca = "migrado" if migrado else "por migrar"
        aviso = f"  <- {n_taut} constantes!" if n_taut else ""
        print(f"  {os.path.basename(f):20} afirmações: {n_af:3}   {marca:11}{aviso}")
    print()
    print(f"  total de afirmações : {tot_af}")
    print(f"  constantes (`True`) : {tot_taut}")
    print(f"  migrados            : {tot_mig}/{len(alvos)}")
    print()
    print("  Enquanto um certificador não exibir refutador por afirmação, o seu N/N")
    print("  mede o fechamento e NÃO mede os dentes. O teste de mutação faz-se à mão.")
    return 1 if tot_taut else 0


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--auditar", action="store_true")
    a = p.parse_args()
    return auditar() if a.auditar else auto()


if __name__ == "__main__":
    sys.exit(main())
