    .cr      6502
    .tf      hello.hex, AP1, 8
    .or      $1000

prchar       .eq $ffef
exitapp      .eq $ff1f

start
    ldx      #0
_again
    lda      ostr, x
    cmp      #0
    beq      _done
    jsr      prchar
    inx
    jmp      _again
_done
    lda      #36                    ; print a $ to indicate the app is done
    jsr      prchar
    jmp      exitapp

ostr
    .as     #$0d,#$0a,"hello david lee!!!",#$0

