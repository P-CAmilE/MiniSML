# Standard ML

交互式，函数式语言

以 ; 隔开

***

## 值的声明

声明（ declaration）就是赋予某个东西一个名字。ML里有许多东西可以赋予名字：**值**、 **类型**、**签名**、**结构**和**函子**。

**值**包括**数**、**字符串**和**函数**。值得名字称作**变量**。

对于SML语言，不能单独地对现有变量进行赋值，只能再使用声明时更改名字的值（即**名字的重新声明**），<u>直接的等式即为真值表达式</u>。

#### 命名常量

关键字：val  格式：`val 常量名 = 常量；`

返回： `> val 常量名 = 常量 -> 类型`

运行环境中顶层的表达式的值被存入 it

#### 声明函数

关键字：fun 格式：`fun 函数名 (形参) = 函数体;` or  `fun 函数名 形参 = 函数体;`

返回： `> val 函数名 = fn : 形参类型 -> 返回值类型`

​	注：其中fn为函数的值

函数的调用：`函数名(实参);` or `函数名 实参;`

#### 注释

以 (* 开始，以 *) 结尾， 可以跨行，可以嵌套。

#### 名字的重新声明

 和命令式语言里面的变量不同，这里的变量是**不能更新的**。不过一个名字可以被重新用作其他的用途。如果一个名字被再次声明，那 么新的含义将在之后被采用（**包括it**），但不会影响现有的对这个名字的使用。

名字的永久性（称作静态绑定， static binding），重新声明一个名字不会损坏系统、库或是你的程序。 在所有地方都可以看到的这些**绑定**（名字与值的结合）被称作环境（environment）。 

#### 标识符

字母名字：字母开头，后面可以跟随字母、数字、下划线 _ 、单引号 ' 。标识符大小区分。

符号名字：由以下字符组成：! % & $ # + - * / : < = > ? @ \ ~ ` ^ |

某些特殊字符组成为ML语法保留： :   |   =   =>   ->   #   :>

#### 声明类型

格式：关键字**type**，`type variable = type(*type...)`

```SML
type νec = real*real;
> type vec
type INT = int;
> type INT
```

***

## 数、字符和真值

#### 算数运算

ML区分整数 int 与实数 real

整数：用 ~ 表示负号，不加修饰表示正数。

整数运算：加法 +，减法 -，乘法 *，除法 div，取模 mod，这些中缀运算符，遵循常规优先级。

实数：一个实数常量包含一个小数点或者E记法，或者两者都有。结尾En的意思是”乘以10的n次方”。负的指数部分是以一元减号（～）开始。负实数以一元减号（～）开始。

实数运算：实敬的中缀运算符包括加法（＋）、减法（－）、乘法（＊） 和除法（／）。遵循常规优先级。

<!--函数应用比中缀运算符优先级更高-->

#### 类型约束

ML可以根据表达式里面用到的函数和常量的类型推导出大多数表达式的类型。 不过某些内置的函数是被**重载 (overloaded)**了的，它们不止有一个含义。例如，＋和＊对于 整数和实数中都有定义。重载函数的类型必须由上下文来确定，偶尔必须显式地指出。

 ML不能确定这个函数是给整数用的，还是实数用的，拒绝接受这样的 声明。

```SML
fun square x = x*x;
> Error-Unable to resolve overloading for  *
```

1.我们可以指定参数的类型：

```SML
fun square (x : real) = x*x;
> val square = fn : real -> real
```

2.也可以指定结果的类型：

```SML
fun square (x) : real = x*x;
> val square = fn : real -> real
```

3.同样，我们可以指定函数体的类型：

```SML
fun square (x) = x*x : real;
> val square = fn : real -> real
```

 **类型约束也可以出现在函数体内部，实际上几乎任何地方都可以。**

#### 字符串和字符

消息和其他文本都是字符组成的串。它们的类型是string。**字符串常量是用双引号括起来。**

```SML
"How now! a rat? Dead, for a ducat, dead !";
> "How now! a rat? Dead, tor a ducat , dead !" : string
```

连接操作符 ^ 将两个字符串首尾相接：

```SML
"Fair " ^ "Ophelia";
>"Fair Ophelia" : string
```

内置函数size返回一个字符串里面的字符个数。

特殊字符：由一个反斜线开始的转义序列（ escape sequence） 可以将某些特殊字符插入到字符串中。

字符： char，一个字符不同于只包含一个字符的字符串。 格式：

`#"a"     #" "     #"\n"` 分别为一个字字母，一个空格，一个特殊字符

**函数ord**和**函数chr**将字符和字符码互相转换。大多数实现都是使用ASCII字符集，如果0<k<255，那么chr(k)返回一个以k为字符码的字符。相应地， ord(c)返回字符c的字符码。

**函数str**和**函数String.sub**将字符和字符串互相转换。 如果c是一个字符的话， str(c)则是相对应的字符串。相应地，如果s是一个字符串的话，那么String.sub(s, n)则返回S里面的第n个字符，从零开始计数。

#### 真值(布尔值)和条件表达式

条件表达式格式：`if E then E1 else E2` 

**测试**：就是一个**类型为bool**的表达式E，取值可以是**true**（真）和**false** （假）。

关系运算：**大于<**，**小于>**，**小于等于 <=**，**大于等于>=**；这些关系在整数和实数上都有定义，它们也可以按字母顺序测试字符串和字符的大小。**相等＝**和**不相等<>** 对于大多数类型都可以用于关系运算。 

布尔运算：**逻辑或 orelse**，**逻辑与 andalso**，**逻辑非 not函数**；返回布尔值的函数也叫做**谓词**(predicate)。

***

 ## 序偶、元组和记录

ML元组的分量本身也可以是元组或任何其他的值。**元组**与**记录**也是**值**。

#### 序偶(二元组)

格式： `(expression,expression)`

```SML
(2.5,~1.2);
> (2.5,~1.2) : real * real
```

向量可以是函数的参数和结果，也可以命名。总的来说，它们拥有和ML中内置类型（比如整数）完全一样的权利。

```SML
fun lengthvec (x,y) = Math.sqrt(x*x + y*y);
> val lengthvec = fn : real * real -> real
```

注：**函数Math.sqrt**只对实数定义，因此约束了被重载得运算符，使其必须使实数类型的。

```SML
fun negvec (x, y) : real*real = (~x,~y)；
> val negvec = fn : real * real -> real * real
```

注：因为负号（~）是被重载的，所以类型约束 real*real 是**必需**的。

#### 多参数和多结果的函数

严格地说，每个ML函数只有一个参数和一个返回结果。

利用元组，函数可以有任意多个参数，以及返回任意多个结果。同时元组的可嵌套性，为参数提供了更多的可能性。

```SML
fun scalevec (r, (x,y)) : vec = (r*x, r*y); 
> val scalevec = fn : real ’ (real * real) -> vec
```

#### 选择元组的分组

在一个模式上，比如(x, y)，定义的函数通过模式变量x和y来引用参数里面的分量。 

```SML
val (xc, yc) = scalevec (4.0, a); 
> val xc = 6.0 : real 
> val yc = 27.2 : real 
```

val声明里的模式也可以使用嵌套。

#### 零元组和单元类型unit

零元组：`()` ， 读作**单元**（unity）

它没有分量。它的用途在于，当没有数据需要传输的时候提供一个占位符。 零无组是unit类型里唯一的一个值。

**过程**：过程就是**返回值类型为unit**类型的“函数”。过程的调用是为了看到它的作用一一而不是得到它的值，反正值总()。

#### 纪录

记录：记录是一种其分量一一称作域(field)一一具有标签的元组。对于元组的有序性，记录是无序的。

格式：`{label=expression,label=expression,...}`

**记录模式**：由形如label = variable(变量)的域构成的记录模式赋予每一个变量相应的标签值。如果我们不需要所有的域，那么可以在其他域所在的地方写三个点(...).

```SML
val {name=nameV, born=bornV, ...} = henryV;
> val bornv = 1387 : int
> val nameV = ”Henry V” : string
```

 通常，我们想要打开一个记录，以便直接看到里面的域。可以用模式label = label来指定每一 个域，<u>使变量和标签同名</u>。这种描述可以简单地缩写为label。

**记录域的选择**：选择符 `# label`可以从记录里取得给定的label的值。

```SML
#quote richardIII;
> ”Plots have I laid ...” : string
```

 n元组(X1, X2, ... , Xn)就是一个使用数作为域标签的记录的简写：`{1 = X1, 2 = X2, ... , n = Xn}`  

同样的，使用选择操作符#k可以提取n元组里面的第k个分量的值。

```SML
#2 (”a”,”b",3，false);
> “b” : string
```

 **部分记录描述**：仅选择一些域而忽略另一些域是不能完全确定记录类型的；一个函数只能定义在完整的记录类型之上。比如，不能定义一个函数让它接受所有具有born和died域的记录，必须指定所有的域标识(通常是利用类型约束)。

**声明记录类型**：

格式：`type variable = {label : type,label : type, ...};`

可以使用记录类型约束函数：

```SML
fun lifetime(k : king) = #died k - #born k;
> val lifetime = fn : king -> int
```

利用模式，lifetime也可以这样声明：

```SML
fun lifetime ({born,died, ... } : king) = died - born;
> val lifetime = fn : king -> int
```



 #### 中缀操作符

**中级操作符（ infix operator）是一个写在它的两个参数中间的函数。**

使用infix指令声明中缀操作符：`infix variable;`

名字首先被指定成中缀，然后才’定义它的值。

```SML
infix xor;
fun (p xor q) = (p orelse q) andelse not (p andalso q);
> val xor = fn : (bool * bool)-> bool
ture xor flase;    (*调用中缀操作符*)
```

**中缀的优先级**： 

`infix`可以声明一个从0到9的优先级（0最低9最高），同时其规定的操作符是左结合的，`infixr`规定右结合。

<!--中缀操作符可以认为是识别了两种标识符（字母与符号）。故在使用的时候注意要将可能引发错误识别的字符用空格隔开-->

**将中缀操作符作为函数写法**：使用关键字`op 中缀操作符`可以覆盖中缀状态，使中缀操作使用普通函数的写法。

```SML
op^ (”Mont” , ”joy”);
>"Montjoy" : string
```

**中缀状态也可以被取消**：使用关键字`nonfix variable`。如果xor是一个中缀操作符的话，那么指令nonfix xor让它回到普通函数的记法。 后面的infix指令又可以将xor变成一个中缀操作符。

------

## 表达式的求值

ML中的传值规则是**传值调用（call-by-value）**或称为**严格（strict）求值**。

#### ML中的求值：传值调用

ML的求值规则基于一个基本想法：为了求得f(E)的值，首先对表达式E进行求值。这个值替换了f函数体内的形式参数，然后函数就可以进一步求值。

ML的求值之所以被称为传值调用，是因为总是将函数参数的值传进函数。

#### 传值调用下的递归函数

对于递归函数，我们改写其一般的书写形式：

源：

```SML
fun fact n =
	if n=O then 1
	else n * fact(n-1);
```

改进：

```SML
fun facti (n,p) = 
	if n=O then p
	else facti(n-1, n*p); 
```

改进后的求值过程是**迭代的**，也称为**尾递归的**。

这样的尾调用（tail call）可以优化执行：将新值赋予参数n和p，然后直接跳回函数开始，避免了正式函数调用的开销。在函数fact中的递归调 用不是尾调用，因为它返回的值要进行进一步地运算，也就是还要乘以n。 

**条件的与和或**： ML的中缀布尔操作符andalso和orelse并不是函数，只代表某类条件表达式。

E1 andalso E2：if E1 then E2 else false 的简式

E1 orelse E2：if E1 then true else E2 的简式

它们仅当需要时才会对E2求值。如果它们是函数的话，传<u>值调用规则会对两个参数都求值</u>。其他ML中缀操作符则的确是函数。 

#### 传需调用(惰性求值)

**传名调用**：如何将参数作为表达式而不是值传进函数：为了计算f(E)的值，直接将E替换进f的函数体。然后，计算所得到的表达式的值。 这就是**传名调用（call-by-name）规则**。

**传需调用**：保证了对每一个参数最多求一次值。它不直接将一个表达式替换进函数体，而是在参数出现的地方用指针连接到表达式上。如果参数指向的表达式曾被求值，那么这个参数出现的地方都共享这个值，这就是**传需调用（call-by-need）**。指针结构构成了关于函数和参数的有向图。当图的一部分被求过值后，图会被结果值所更新。这称为**图归约（ graph reduction）**。

------

## 书写递归函数

#### 整数的次幂

#### 斐波那契数列

#### 整数平方根

---

## 局部声明

#### let表达式

**let表达式**： `let D in E end`

[^D]: 声明，可以是复合声明
[^E]: 表达式

ML允许在一个表达式里面声明几个名字，在求值过程中，首先对声明D进行求值，对声明中的表达式进行求值，并且将其结果命名。这样建立起来的环境仅在let表达式中是可见的。然后再对表达式E进行求值，它的值作为整个表达式的值返回。

```SML
fun fraction (n, d) = 
	let val com = gcd(n,d) 
	in (n div com , d div com) end; 
> val fraction = fn : int * int - > int * int 
```

 通常D是复合声明，也就是说包含一系列的声明：`D1;D2;...;Dn`（其中分号是可选的）

**嵌套的函数声明**：在let表达式中，D部分可以是声明一个函数，形成嵌套的函数。

```SML
fun sqroot a =
 	let val acc = 1.OE~10
		fun findroot x =
			let val nextx = (a/x + x) / 2.0
			in if abs (x-nexrx) < acc*x
				then nextx else findroot nextx
			end
	in findroot 1.0 end;
> val sqroot = fn : real -> real 
```

#### 使用local来隐藏声明

**local声明**：`local D1 in D2 end`

[^D1，D2]: 均为声明，也均可以为复合申明

在该声明中D1只在D2内可见，在外面不可见。local的唯一用途就是隐藏声明。

```SML
local 
	fun itfib (n, preν, curr) : int =
    	if n=l then curr 
    	else itfib (n-1, curr, prev+curr)
in
	fun fib (n) = itfib(n,0,1)
end;
> val fib = fn : int -> int
```

#### 联合声明

**联合声明**：`val Id1 = E1 and … and Idn = En `

一个联立声明同时定义多个名字。通常声明是彼此独立的。但是fun声明允许递归，因此联立声明可以引入相互递归的函数。**首先对表达式E1, ... , En求值，然后声明标识符ld1, ...，Idn，让它们具有相应的值**。由于声明在所有表达式求值结束之前还不起作用，所以它们的顺序是无关紧要的。

------

## 模块系统初步

#### 结构

结构中声明（包括复合声明）被包含在关键字**struct**和**end**之间，这个组合的结果可以通过structure结构声明绑定到一个标识符上。

```SML
structure Complex = 
	struct
	type t = real*real;
	val zero = (0.0, 0.0);
	fun sum ((x,y),(x',y')) = (x+x', y+y') : t;
	fun diff ((x,y),(x',y')) = (x-x', y-y') : t;
	fun prod ((x,y),(x',y')) = (x*x' - y*y', x*y' + x'*y) : t;
	fun recip (x,y) = 
		let val t = x*x + y*y
		in (x/t, ~y/t) end;
	fun quo (z, z') = prod(z,recip z');
	end;
```

当结构Complex可见时，其组件可以通过**复合名字**来访问，例Complex.zero。在结构体内，组件通过原有的标识符访问。

结构应该被看作是一种封装了的环境。

#### 签名

签名是结构里面每一个组件的描述。在对结构作出声明之后，ML通过打印出它所导出的相应的签名来作为回应。

签名中由关键字**sig**和**end**括住了签名体。通过声明我们自己的签名，同时去掉那些私有的名字，就可以将它们隐藏起来，而不被结构的使用者看到。

```SML
signature ARITH =
	sig
	type t
    val zero : t
    val sum : t * t -> t
    val diff: t * t -> t
    val prod: t * t -> t
    val quo : t * t -> t
    end; 
```

我们可以在定义结构的时候使他遵守签名，`stucture variable : signaturename = ...`

------

## 多态类型检查

#### 类型推导

#### 多态函数声明

------

## 表





