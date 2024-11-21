# Коллоквиума МЛиТА-2024
## Задание 1
В задании 1 выведены все аксиомы с использованием только первых 3-х аксиом, правила `modus ponens` и теоремы о дедукции. Примеры вывода указанных аксиом записаны в папке `conclusions`.

## Задание 2
## Задание 3
## Задание 4

Выражение для других команд:

```
(a>((b>((c>d)>((c>d)*(c>c))))>!a))>!a
```

То как выводится в нашей программе

```
input: (a>((b>((c>d)>((c>d)*(c>c))))>!a))>!a
normalized input: (a>((b>((c>d)>((c>d)*(c>c))))>!a))>!a

deduction theorem: Γ ⊢ (a>((b>((c>d)>((c>d)*(c>c))))>!a))>!a <=> Γ U {a>((b>((c>d)>((c>d)*(c>c))))>!a)} ⊢ !a
1. axiom: (A>B)>((C>B)>((!A>C)>B))
2. axiom: A>(B>A)
3. axiom: (A>(B>C))>((A>B)>(A>C))
4. axiom: A>(B>(A*B))
5. axiom: (!A>!B)>((!A>B)>A)
6. mp(2,2): A>(B>(C>B))
7. mp(3,3): ((A>(B>C))>(A>B))>((A>(B>C))>(A>C))
8. mp(4,2): A>(B>(C>(B*C)))
9. mp(2,3): (A>B)>(A>A)
10. mp(4,3): (A>B)>(A>(A*B))
11. mp(6,7): (A>((B>A)>C))>(A>C)
12. mp(5,2): A>((!B>!C)>((!B>C)>B))
13. mp(8,7): (A>((B>(A*B))>C))>(A>C)
14. mp(5,9): (!A>!B)>(!A>!B)
15. mp(9,10): (A>B)>((A>B)*(A>A))
16. mp(6,11): A>(B>(C>A))
17. mp(12,13): A>((!B>(A>B))>B)
18. mp(14,1): (A>(!B>!C))>(((!B*C)>A)>(!B>!C))
19. mp(15,16): A>(B>((C>D)>((C>D)*(C>C))))
20. mp(17,18): (((A>(B>!A))*A)>B)>((A>(B>!A))>!A)
21. mp(19,20): (A>((B>((C>D)>((C>D)*(C>C))))>!A))>!A
change variables: (A>((B>((C>D)>((C>D)*(C>C))))>!A))>!A
D -> d
C -> c
B -> b
A -> a
proved: (a>((b>((c>d)>((c>d)*(c>c))))>!a))>!a
```

Затраченное время:
13 секунд
