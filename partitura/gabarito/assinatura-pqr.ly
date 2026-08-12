% trial (p,q,r)=(2,2,0) --- excerto; preferir ./emite tradutor
\version "2.24.0"
\language "english"
\header { title = "(p,q,r)=(2,2,0)" tagline = ##f }
global = { \key c \major \time 4/4 \tempo 4 = 66 }
vozP = \absolute { \global \clef treble
  c'4 d'4 e'4 f'4 |
  g'4 a'4 b'4 c''4 \bar "|."
}
vozQ = \absolute { \global \clef treble
  c''4 b'4 a'4 g'4 |
  f'4 e'4 d'4 c'4 \bar "|."
}
vozR = \absolute { \global \clef treble
  r1 | r1 | r1 | r1 \bar "|."
}
\score { \new StaffGroup <<
  \new Staff \with { instrumentName = #"p (+)" } { \vozP }
  \new Staff \with { instrumentName = #"q (-)" } { \vozQ }
  \new Staff \with { instrumentName = #"r (0)" } { \vozR }
>> \layout { } }
