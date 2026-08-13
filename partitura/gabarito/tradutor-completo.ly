% gerado por partitura/emite.c --- PARTITURA COMPLETA DO TRADUTOR
% Pi_tradutor |-> Pi_musical: Peano/estelar (nu, trial, pi_k, Lei 5/6/8, Hurwitz).
% NAO inventa informacao fora da assinatura.
% SementeEstrela: Dim=8 Alcance=3 Lado=0 Interface=6 bpm=72 compassos=8
\version "2.24.0"
\language "english"
\header {
  title = "Tradutor --- partitura completa"
  subtitle = "Pi: Dim=8 Alcance=3 Lado=0 Iface=6"
  composer = "tiffany / emite.c"
  tagline = ##f
}
global = {
  \key c \major
  \time 4/4
  \tempo "Metronomo" 4 = 72
}

metronomo = \absolute {
  \global \clef percussion
  \override Staff.StaffSymbol.line-count = #1
  c'4 c'4 c'4 c'4 |
  c'4 c'4 c'4 c'4 |
  c'4 c'4 c'4 c'4 |
  c'4 c'4 c'4 c'4 |
  c'4 c'4 c'4 c'4 |
  c'4 c'4 c'4 c'4 |
  c'4 c'4 c'4 c'4 |
  c'4 c'4 c'4 c'4 \bar "|."
}
batuta = \absolute {
  \global \clef bass
  c4 r4 e'4 r4 |
  r4 e'4 r4 c4 |
  e'4 r4 c4 r4 |
  r4 c4 r4 e'4 |
  c4 r4 e'4 r4 |
  r4 e'4 r4 c4 |
  e'4 r4 c4 r4 |
  r4 c4 r4 e'4 \bar "|."
}
maestro = \absolute {
  \global \clef treble
  g'1 |
  g'1 |
  g'1 |
  g'1 |
  g'1 |
  g'1 |
  g'1 |
  g'1 \bar "|."
}
cordas = \absolute {
  \global \clef treble
  c'4 d'4 e'4 f'4 |
  c'4 d'4 e'4 f'4 |
  c'4 d'4 e'4 f'4 |
  c'4 d'4 e'4 f'4 |
  c'4 d'4 e'4 f'4 |
  c'4 d'4 e'4 f'4 |
  c'4 d'4 e'4 f'4 |
  c'4 d'4 e'4 f'4 \bar "|."
}
madeiras = \absolute {
  \global \clef treble
  e''4 g''4 e''4 c''4 |
  g''4 e''4 c''4 e''4 |
  e''4 c''4 e''4 g''4 |
  c''4 e''4 g''4 e''4 |
  e''4 g''4 e''4 c''4 |
  g''4 e''4 c''4 e''4 |
  e''4 c''4 e''4 g''4 |
  c''4 e''4 g''4 e''4 \bar "|."
}
metais = \absolute {
  \global \clef treble
  c'4 e'4 af'4 f'4 |
  c'4 e'4 af'4 f'4 |
  c'4 e'4 af'4 f'4 |
  c'4 e'4 af'4 f'4 |
  c'4 e'4 af'4 f'4 |
  c'4 e'4 af'4 f'4 |
  c'4 e'4 af'4 f'4 |
  c'4 e'4 af'4 f'4 \bar "|."
}
percussao = \absolute {
  \global \clef bass
  c4 r4 g4 r4 |
  c4 r4 g4 r4 |
  c4 r4 g4 r4 |
  c4 r4 g4 r4 |
  c4 r4 g4 r4 |
  c4 r4 g4 r4 |
  c4 r4 g4 r4 |
  c4 r4 g4 r4 \bar "|."
}
leiOito = \absolute {
  \global \clef treble
  c'4 d'4 e'4 f'4 |
  g'4 a'4 b'4 c''4 |
  c'4 d'4 e'4 f'4 |
  g'4 a'4 b'4 c''4 |
  c'4 d'4 e'4 f'4 |
  g'4 a'4 b'4 c''4 |
  c'4 d'4 e'4 f'4 |
  g'4 a'4 b'4 c''4 \bar "|."
}

\score {
  \new StaffGroup <<
    \new Staff \with { instrumentName = #"Metronomo" shortInstrumentName = #"Metr." }
      { \metronomo }
    \new Staff \with { instrumentName = #"Batuta" shortInstrumentName = #"Bat." }
      { \batuta }
    \new Staff \with { instrumentName = #"Maestro" shortInstrumentName = #"Mae." }
      { \maestro }
    \new Staff \with { instrumentName = #"Cordas (1)" shortInstrumentName = #"Cor." }
      { \cordas }
    \new Staff \with { instrumentName = #"Madeiras (2)" shortInstrumentName = #"Mad." }
      { \madeiras }
    \new Staff \with { instrumentName = #"Metais (4)" shortInstrumentName = #"Met." }
      { \metais }
    \new Staff \with { instrumentName = #"Percussao (8)" shortInstrumentName = #"Per." }
      { \percussao }
    \new Staff \with { instrumentName = #"Lei 8" shortInstrumentName = #"L8" }
      { \leiOito }
  >>
  \layout {
    \context { \StaffGroup \consists "Span_bar_engraver" }
  }
  \midi { }
}
