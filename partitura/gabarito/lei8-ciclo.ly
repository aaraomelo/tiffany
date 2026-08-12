%% excerto Lei 8 --- preferir: ./emite tradutor
\version "2.24.0"
\language "english"
\header { title = "Lei 8" tagline = ##f }
global = { \key c \major \time 4/4 \tempo 4 = 72 }
leiOito = \absolute {
  \global \clef treble
  c'4 d'4 e'4 f'4 |
  g'4 a'4 b'4 c''4 \bar "|."
}
\score { \new Staff \with { instrumentName = #"Lei 8" } { \leiOito } \layout { } }
