%% excerto naipes --- preferir: ./emite tradutor
\version "2.24.0"
\language "english"
\header { title = "Naipes Hurwitz" tagline = ##f }
global = { \key c \major \time 4/4 \tempo 4 = 72 }
cordas = \absolute {
  \global \clef treble
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
  c''4 e''4 g''4 e''4 \bar "|."
}
metais = \absolute {
  \global \clef treble
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
  c4 r4 g4 r4 \bar "|."
}
\score { \new StaffGroup <<
  \new Staff \with { instrumentName = #"Cordas (1)" } { \cordas }
  \new Staff \with { instrumentName = #"Madeiras (2)" } { \madeiras }
  \new Staff \with { instrumentName = #"Metais (4)" } { \metais }
  \new Staff \with { instrumentName = #"Percussao (8)" } { \percussao }
>> \layout { } }
