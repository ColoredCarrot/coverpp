const
  ModPathName = 'modifypath';
  ModPathType = 'system';

function ModPathDir(): TArrayOfString;
begin
  SetArrayLength(Result, 1);
  Result[0] := ExpandConstant('{app}\bin');
end;

#include "modpath.iss"
