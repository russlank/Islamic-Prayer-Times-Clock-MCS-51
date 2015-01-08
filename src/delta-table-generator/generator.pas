var F: Text;
    A1, A2, A3, A4: array[1..365] of Integer;
    I: Integer;

    V1: Word;
    V2,
    V3,
    V4,
    V5,

    V6,
    V7,
    V8,
    V9,
    V10: Integer;

begin
     Assign(F, 'TABLE.TXT');
     Reset(F);
     for I := 1 to 365
     do begin
        Write(I,'      ',#13);
        Read(F,A3[I],A4[I]);
        Read(F,A1[I],A2[I]);
        ReadLn(F);
        if EOF(F)
        then Break;
        end;
     Close(F);
     Assign(F,'TABLE.C');
     Rewrite(F);
     V2 := 0;
     V3 := 0;
     V4 := 0;
     V5 := 0;
     V6 := 0;
     V7 := 0;
     V8 := 0;
     V9 := 0;
     for I := 2 to 365
     do begin
        WriteLn(F,A1[I] - A1[I - 1]:10,A2[I] - A2[I - 1]:4,' // ',A4[I]:2,'\',A3[I]:2,'    ',I);
        if (A1[I] - A1[I - 1] > V2)
        then begin
             V10 := V6;
             V6 := V2;
             V2 := A1[I] - A1[I - 1];
             end;
        if (A1[I] - A1[I - 1] < V3)
        then begin
             V7 := V3;
             V3 := A1[I] - A1[I - 1];
             end;
        if (A2[I] - A2[I - 1] > V4)
        then begin
             V8 := V4;
             V4 := A2[I] - A2[I - 1];
             end;
        if (A2[I] - A2[I - 1] < V5)
        then begin
             V9 := V5;
             V5 := A2[I] - A2[I - 1];
             end;
        end;

     WriteLn(F,'----------------------------');
     WriteLn(F,'1. MAX = ',V2,' ',V6,' ',V10,'   MIN = ', V3,' ',V7);
     WriteLn(F,'2. MAX = ',V4,' ',V8,'   MIN = ', V5,' ',V9);
     WriteLn(F);

     WriteLn(F,'DELTA TABLE 1');
     for I := 2 to 365
     do WriteLn(F,(A1[I] - A1[I - 1] - V3):10,', // ',A4[I]:2,'\',A3[I]:2,'    ',I);
     WriteLn(F);
     WriteLn(F);
     WriteLn(F);
     WriteLn(F,'DELTA TABLE 2');
     for I := 2 to 365
     do WriteLn(F,(A2[I] - A2[I - 1] - V5):10,', // ',A4[I]:2,'\',A3[I]:2,'    ',I);
     WriteLn(F);
     WriteLn(F);
     WriteLn(F);

     for I := 1 to 365
     do begin
        WriteLn(F,A1[I]:10,', // ',A4[I]:2,'\',A3[I]:2);
        end;
     WriteLn(F);
     WriteLn(F);
     WriteLn(F);
     for I := 1 to 365
     do begin
        WriteLn(F,A2[I]:10,', // ',A4[I]:2,'\',A3[I]:2);
        end;
     WriteLn(F);
     WriteLn(F);
     WriteLn(F);
     for I := 1 to 365
     do begin
        V1 := (((A1[I] + 2350) shl 5) + (A2[I] + 17));
        V2 := (V1 shr 5) - 2350;
        V3 := (V1 and $001F) - 17;

        Write(F,V1:10,', // ',V2:6,',',V3:4,'   ',A4[I]:2,'\',A3[I]:2);
        if ((V2 <> A1[I]) or (V3 <> A2[I])) then WriteLn(F,'<-----ERROR', A1[I]:10,A2[I]:10)
        else WriteLn(F);
        end;
     Close(F);
end.