## 概念

### 存储过程

存储过程是一些预编译的SQL语句。

更加直白的理解：

存储过程可以说是一个记录集，它是由一些SQL语句组成的代码块，这些SQL语句代码像一个方法一样实现一些功能（对单表或多表的增删改查），然后再给这个代码块取一个名字，在用到这个功能的时候调用它就行了（存储过程就像编程语言里的函数）。

- 存储过程是一个预编译的代码块，执行效率比较高；
- 一个存储过程可以替代大量的SQL语句 ，可以降低网络通信量，提高通信速率；
- 可以一定程度上确保数据安全（所有开发人员使用同一个存储过程，则所使用的代码是相同的）；

### 视图

视图是一种虚拟的表，视图通常是一个表或者多个表的行或列的子集，提供 SELECT 语句的封装。可以用来简化数据处理和保护基础数据。

- 只暴露部分字段给访问者，所以就建一个虚表，就是视图。
- 查询的数据来源于不同的表，而查询者希望以统一的方式查询，这样也可以建立一个视图，把多个表查询结果联合起来，查询者只需要直接从视图中获取数据，不必考虑数据来源于不同表所带来的差异   

### 索引与键

索引是对数据库表中一列或多列的值进行排序的结构，是帮助MySQL高效获取数据的数据结构。

你也可以这样理解：索引就是加快检索表中数据的方法。数据库的索引类似于书籍的索引。在书籍中，索引允许用户不必翻阅完整个书就能迅速地找到所需要的信息。在数据库中，索引也允许数据库程序迅速地找到表中的数据，而不必扫描整个数据库。 

**MySQL数据库几个基本的索引类型：**

- 普通索引：也即不加任何限制的索引
- 唯一索引： 不允许具有索引值相同的行，从而禁止重复的索引或键值。系统在创建该索引时检查是否有重复的键值，并在每次使用 INSERT 或 UPDATE 语句添加数据时进行检查。 
- 主键索引： 数据库表经常有一列或列组合，其值唯一标识表中的每一行。该列称为表的主键。它和唯一索引的共性在于都不允许有重复记录，区别在于，唯一索引是不限制null的，也就是说或可以有一条以上的null值插入，但是主键却限定不能为空。 

**索引的优点:**

- 大大加快数据的检索速度; 　　

- 创建唯一性索引，保证数据库表中每一行数据的唯一性; 　　

- 加速表和表之间的连接; 　　

- 在使用分组和排序子句进行数据检索时，可以显著减少查询中分组和排序的时间。

**索引的缺点：**

- 索引需要占物理空间。 　　

- 当对表中的数据进行增加、删除和修改的时候，索引也要动态的维护，降低了数据的维护速度。

**使用索引查询一定能提高查询的性能吗？**

通常,通过索引查询数据比全表扫描要快.但是我们也必须注意到它的代价.

索引需要空间来存储，也需要定期维护， 每当有记录在表中增减或索引列被修改时，索引本身也会被修改。 这意味着每条记录的INSERT,DELETE,UPDATE将为此多付出4,5 次的磁盘I/O.  因为索引需要额外的存储空间和处理,那些不必要的索引反而会使查询反应时间变慢.使用索引查询不一定能提高查询性能,索引范围查询(INDEX  RANGE SCAN)适用于两种情况:

- 基于一个范围的检索,一般查询返回结果集小于表中记录数的30%
- 基于非唯一性索引的检索

 **建立索引时的注意事项：**

- 考虑已在表上创建的索引数量。最好避免在单个表上有很多索引

-  检查已在表上创建的索引的定义。最好避免包含共享列的重叠索引

- 检查某列中唯一数据值的数量，并将该数量与表中的行数进行比较。比如如果有1000w记录，某字段为性别，只有男，女。也就是说一半的记录都是重复的，这样就要考虑是否还有必要建立索引了。

**键**

 数据库中的键（key）又称为关键字，是关系模型中的一个重要概念，它是逻辑结构，不是数据库的物理部分。

- 唯一键：即一个或者一组列，其中没有重复的记录，可以唯一标示一条记录，但可空。

- 主键：属于唯一键，是一个比较特殊的唯一键。区别在于主键不可为空。

- 外键：如果公共关键字在一个关系中是主关键字，那么这个公共关键字被称为另一个关系的外键。由此可见，外键表示了两个关系之间的联系。

**主键索引和主键有什么关系？**

主键索引是创建主键的时候系统自动创建的索引，主键要求不重复，不为空，但是他如何判断有效率呢？当然是建索引了，老是全表遍历还不疯掉。

所以建立主键会自动的建立主键索引。

主键和唯一键的区别在于唯一键可以为空，主键不可以

### 事务

事务（Transaction）是**并发控制的基本单位**。所谓的事务，它是一个操作序列，这些操作要么都执行，要么都不执行，它是一个不可分割的工作单位。

例如，银行转账工作：从一个账号扣款并使另一个账号增款，这两个操作要么都执行，要么都不执行。所以，应该把它们看成一个事务。事务是数据库维护数据一致性的单位，在每个事务结束时，都能保持数据一致性。

针对上面的描述可以看出，事务的提出主要是为了解决并发情况下保持数据一致性的问题。

#### 事务的基本要素（4个基本特征）

●   **Atomic（原子性）**：事务中包含的操作被看做一个逻辑单元，这个逻辑单元中的操作要么全部成功，要么全部失败。

●   **Consistency（一致性）**：只有合法的数据可以被写入数据库，否则事务应该将其回滚到最初状态。

●   **Isolation（隔离性）**：事务允许多个用户对同一个数据进行并发访问，而不破坏数据的正确性和完整性。同时，并行事务的修改必须与其他并行事务的修改相互独立。

●   **Durability（持久性）**：事务结束后，事务处理的结果必须能够得到固化。

#### 事务的并发问题

- 脏读：事务B修改数据但未提交，事务A读数据，然后B回滚，则A读到的是脏数据。
- 不可重复读：事务A第一次读取数据，事务B修改数据提交，事务A第二次读数据，两次数据不一致。
- 幻读：事务A update表的全部行，事务B插入一行，事务A就会发现表中还有未修改的行。（一般加间隙锁）

#### MySQL事务隔离级别

| 事务隔离级别 | 脏读 | 不可重复读 | 幻读 |
| ------------ | ---- | ---------- | ---- |
| 读未提交     | 会   | 会         | 会   |
| 读已提交     | 不会 | 会         | 会   |
| 可重复读     | 不会 | 不会       | 会   |
| 串行话       | 不会 | 不会       | 不会 |

查看mysql的默认事务隔离级别“show global variables like ‘tx_isolation’; ”

### 数据库的乐观锁和悲观锁

数据库管理系统（DBMS）中的并发控制的任务是确保在多个事务同时存取数据库中同一数据时不破坏事务的隔离性和统一性以及数据库的统一性。  

乐观并发控制(乐观锁)和悲观并发控制（悲观锁）是并发控制主要采用的技术手段。

- 悲观锁：假定会发生并发冲突，屏蔽一切可能违反数据完整性的操作
- 乐观锁：假设不会发生并发冲突，只在提交操作时检查是否违反数据完整性。

### 三个范式

第一范式（1NF）：数据库表中的字段都是单一属性的，不可再分。这个单一属性由基本类型构成，包括整型、实数、字符型、逻辑型、日期型等。

第二范式（2NF）：数据库表中不存在非关键字段对任一候选关键字段的部分函数依赖（部分函数依赖指的是存在组合关键字中的某些字段决定非关键字段的情况），也即所有非关键字段都完全依赖于任意一组候选关键字。    

第三范式（3NF）：在第二范式的基础上，数据表中如果不存在非关键字段对任一候选关键字段的传递函数依赖则符合第三范式。所谓传递函数依赖，指的是如 果存在"A → B → C"的决定关系，则C传递函数依赖于A。因此，满足第三范式的数据库表应该不存在如下依赖关系： 关键字段 → 非关键字段 x → 非关键字段 y 。

**三大范式通俗解释：**

（1）简单归纳：

　　第一范式（1NF）：字段不可分；
　　第二范式（2NF）：有主键，非主键字段依赖主键；
　　第三范式（3NF）：非主键字段不能相互依赖。

（2）解释：

　　1NF：原子性。 字段不可再分,否则就不是关系数据库;；
　　2NF：唯一性 。一个表只说明一个事物；
　　3NF：每列都与主键有直接关系，不存在传递依赖。

[数据库设计的三大范式通俗解释](https://www.cnblogs.com/hezhiyao/p/7407468.html)

### 高并发

指在极短单位时间内，极多个请求同时访问服务器。

需要了解大数据高并发的瓶颈在哪里，一般都是数据库层面的，机械硬盘承载不起非常快速的读写操作，cpu承载不起大量的逻辑运算，所以最基本的解决思路就是：

- 换固态硬盘加快硬盘的读写效率。
- 建立缓存中间件降低对硬盘的读写次数，缓存不用多说了，最最最基本和重要的优化策略。
- 将硬盘的读写或者数据的计算分摊到多台机器上，也就是集群。hadoop就是基于这个层面的。
- 良好的查询算法，降低读的次数，分表，分库，索引等都是基于这层面的。

理论上来讲，在带宽充裕的情况下，只要遵循上面的4个思路进行延伸就可以解决大部分的高并发问题。

## 操作

### 检索不同的行 DISTINCT

**DISTINCT：返回某一列不同的值**

```mysql
mysql> SELECT DISTINCT order_num FROM orderitems;
+-----------+
| order_num |
+-----------+
|     20005 |
|     20006 |
|     20007 |
|     20008 |
|     20009 |
+-----------+

#DISTINCT 应用于所有列而不仅仅是前置它的列，
#就是说当某一行 order_num 与 order_item 都一样，那么就过滤掉这一行
mysql> SELECT DISTINCT order_num,order_item FROM orderitems;
+-----------+------------+
| order_num | order_item |
+-----------+------------+
|     20005 |          1 |
|     20005 |          2 |
|     20009 |          4 |
|     20005 |          4 |
|     20009 |          1 |
|     20008 |          1 |
|     20006 |          1 |
|     20009 |          2 |
|     20009 |          3 |
|     20005 |          3 |
|     20007 |          1 |
+-----------+------------+

#下面的语句有问题
mysql> SELECT DISTINCT order_num, DISTINCT order_item FROM orderitems;
```

### 限制结果 LIMIT

```mysql
# 查找 prod_name 的前 5 行，从索引为 0 行开始的后 5 行（行数的序号从 0 开始）
mysql> SELECT prod_name FROM products LIMIT 5;

# 查找 prod_name 从索引为 5 的行开始的后 5 行
mysql> SELECT prod_name FROM products LIMIT 5, 5;
```

### 排序 ORDER BY
```mysql
mysql> SELECT vend_id, vend_name FROM vendors ORDER BY vend_name;

# 按多个列排序，先按州排序，再按城市排序（注意 NULL 会排在前面）
mysql> SELECT vend_id, vend_name, vend_city, vend_state FROM vendors ORDER BY vend_state, vend_city;

# 指定排序方向，按价格降序，按名称升序
mysql> SELECT prod_id, prod_price, prod_name FROM products ORDER BY prod_price DESC,prod_name;

# 价格和名称都降序
mysql> SELECT prod_id, prod_price, prod_name FROM products ORDER BY prod_price DESC,prod_name DESC;

```

### 过滤数据 WHERE
```mysql
# 注意不等于 <>，也可以用 !=，
# 选择在某两个值之间的【范围值检查】 BETWEEN

# 选择价格在闭区间 [5, 10] 的商品
mysql> SELECT prod_name, prod_price FROM products WHERE prod_price BETWEEN 5 AND 10;
+----------------+------------+
| prod_name      | prod_price |
+----------------+------------+
| .5 ton anvil   |       5.99 |
| 1 ton anvil    |       9.99 |
| Bird seed      |      10.00 |
| Oil can        |       8.99 |
| TNT (5 sticks) |      10.00 |
+----------------+------------+

# 空值检查
# 选择没有邮箱的顾客 id、名称
mysql> SELECT cust_id, cust_name FROM customers WHERE cust_email IS NULL;
+---------+-------------+
| cust_id | cust_name   |
+---------+-------------+
|   10002 | Mouse House |
|   10005 | E Fudd      |
+---------+-------------+

# 注意
# is 仅用在 is null 或 is not null；
# = 用在 2 种情况下：一是判断值是否相等 where id = 1；二是用于赋值 set id = 1。 
```

### 指定条件范围 IN

```mysql
# 查找供应商 1001，1005 的商品名、价格，按升序显示
mysql> SELECT prod_name, prod_price FROM products
    -> WHERE vend_id IN (1001, 1005)
    -> ORDER BY prod_name;
+--------------+------------+
| prod_name    | prod_price |
+--------------+------------+
| .5 ton anvil |       5.99 |
| 1 ton anvil  |       9.99 |
| 2 ton anvil  |      14.99 |
| JetPack 1000 |      35.00 |
| JetPack 2000 |      55.00 |
+--------------+------------+

# NOT 操作符，只有一个功能否定它后面的任何条件
# 查找除供应商 1001，1005 的商品名、价格，按升序显示
mysql> SELECT prod_name, prod_price FROM products
    -> WHERE vend_id NOT IN (1001, 1005)
    -> ORDER BY prod_name;
```

**下面的语句跟上面的一样，但IN有很多优点**

- IN 操作语法清晰直观，计算次序更容易管理
- IN 操作一般比 OR 操作执行更快
- IN 的最大优点是可以包含其他 SELECT 语句

```mysql
mysql> SELECT prod_name, prod_price FROM products
    -> WHERE vend_id = 1001 OR vend_id = 1005
    -> ORDER BY prod_name;
```

### 通配符过滤

```
# 检索商品名字包含 'jet' 的商品
# 百分比 % 表示任何字符串出现任何次数
mysql> SELECT prod_id, prod_name FROM products WHERE prod_name LIKE '%jet%';
+---------+--------------+
| prod_id | prod_name    |
+---------+--------------+
| JP1000  | JetPack 1000 |
| JP2000  | JetPack 2000 |
+---------+--------------+

# 搜索名称以 s 开头 以 e 结尾的商品
mysql> SELECT prod_id, prod_name FROM products WHERE prod_name LIKE 's%e';
+---------+-----------+
| prod_id | prod_name |
+---------+-----------+
| SAFE    | Safe      |
+---------+-----------+

# 下划线 '_' 只能匹配单个字符，而不是多个字符，也不能匹配 0 个。
```

### 正则表达式

### 计算字段

```mysql
# 想让查询的结果显示为 vend_name(vend_country) 格式，使用 AS vend_title 给该字段取别名
mysql> SELECT Concat(vend_name, '(', vend_country, ')') AS vend_title
    -> FROM vendors
    -> ORDER BY vend_name;
+------------------------+
| vend_title             |
+------------------------+
| ACME(USA)              |
| Anvils R Us(USA)       |
| Furball Inc.(USA)      |
| Jet Set(England)       |
| Jouets Et Ours(France) |
| LT Supplies(USA)       |
+------------------------+

# 执行算术运算
mysql> SELECT prod_id, quantity, item_price, quantity * item_price AS expanded_price
    -> FROM orderitems WHERE order_num = 20005;
+---------+----------+------------+----------------+
| prod_id | quantity | item_price | expanded_price |
+---------+----------+------------+----------------+
| ANV01   |       10 |       5.99 |          59.90 |
| ANV02   |        3 |       9.99 |          29.97 |
| TNT2    |        5 |      10.00 |          50.00 |
| FB      |        1 |      10.00 |          10.00 |
+---------+----------+------------+----------------+
```

### 使用数据处理函数

**文本处理**

- Upper()
- Lower()

**日期和时间处理函数**

- Date()     返回日期之间的时间部分
- Year()  Month()  Day()     分别返回日期的年、月、日
- Time()     返回日期的时间部分
- Hour()  Minute()  Second()    分别返回日期的时、分、秒

```mysql
# 返回 2005-09-01 的订单信息
mysql> SELECT cust_id, order_num, order_date FROM orders WHERE Date(order_date) = '2005-09-01';
+---------+-----------+---------------------+
| cust_id | order_num | order_date          |
+---------+-----------+---------------------+
|   10001 |     20005 | 2005-09-01 00:00:00 |
+---------+-----------+---------------------+

# 返回 '2005-09-01' 与 '2005-09-30' 之间（含）的所有订单信息
mysql> SELECT cust_id, order_num, order_date FROM orders
    -> WHERE Date(order_date) BETWEEN '2005-09-01' AND '2005-09-30';
+---------+-----------+---------------------+
| cust_id | order_num | order_date          |
+---------+-----------+---------------------+
|   10001 |     20005 | 2005-09-01 00:00:00 |
|   10003 |     20006 | 2005-09-12 00:00:00 |
|   10004 |     20007 | 2005-09-30 00:00:00 |
+---------+-----------+---------------------+

# 返回 2005 年 9 月份的所有订单记录（这样不用考虑一个月是大月还是小月，是否闰年问题）
mysql> SELECT cust_id, order_num, order_date FROM orders
    -> WHERE Year(order_date) = 2005 AND Month(order_date) = 9;
+---------+-----------+---------------------+
| cust_id | order_num | order_date          |
+---------+-----------+---------------------+
|   10001 |     20005 | 2005-09-01 00:00:00 |
|   10003 |     20006 | 2005-09-12 00:00:00 |
|   10004 |     20007 | 2005-09-30 00:00:00 |
+---------+-----------+---------------------+
3 rows in set (0.00 sec)
```

### 聚集函数

- AVG()    返回某列的均值
- COUNT()    返回某列的行数
- MAX()  Min()    返回某列的最大值、最小值
- SUM()    返回某列值之和   

```mysql
# 计算商品的数量，价格和以及均价（注意 AS 用法）
# 注意 COUNT(*) 对表中行的数目统计。不管行中包含的是空值还是非空值
# COUNT(column) 对特定列中具有值的行进行统计，不包含 NULL 值
mysql> SELECT COUNT(*), SUM(prod_price), AVG(prod_price) AS '均价' FROM products;
+----------+-----------------+-----------+
| COUNT(*) | SUM(prod_price) | 均价      |
+----------+-----------------+-----------+
|       14 |          225.87 | 16.133571 |
+----------+-----------------+-----------+

# 查找供应商 1003 提供的价格不同的商品的均价
mysql> SELECT AVG(DISTINCT prod_price) AS avg_price FROM products WHERE vend_id = 1003;
+-----------+
| avg_price |
+-----------+
| 15.998000 |
+-----------+

# 统计不同供应商的个数
mysql> SELECT COUNT(DISTINCT vend_id) AS num_of_vend FROM products;
+-------------+
| num_of_vend |
+-------------+
|           4 |
+-------------+

# 统计商品数量、最小、最大价格、平均价格
mysql> SELECT COUNT(*) AS num_items,
    -> MIN(prod_price) AS price_min,
    -> MAX(prod_price) AS price_max,
    -> AVG(prod_price) AS price_avg
    -> FROM products;
+-----------+-----------+-----------+-----------+
| num_items | price_min | price_max | price_avg |
+-----------+-----------+-----------+-----------+
|        14 |      2.50 |     55.00 | 16.133571 |
+-----------+-----------+-----------+-----------+
```

### 数据分组 GROUP BY

```mysql
# 返回每一个供应商提供的商品数量
mysql> SELECT vend_id, COUNT(vend_id) AS num_vend_id FROM products GROUP BY vend_id;
+---------+-------------+
| vend_id | num_vend_id |
+---------+-------------+
|    1001 |           3 |
|    1002 |           2 |
|    1003 |           7 |
|    1005 |           2 |
+---------+-------------+
```

### 过滤分组 HAVING

**HAVING 后面似乎常跟着聚集函数**

```mysql
# 查找单价在区间[10, 50]的商品数量大于 2 的供应商的 id，数量
mysql> SELECT vend_id, COUNT(vend_id) AS num_vend FROM products
    -> WHERE prod_price BETWEEN 10 AND 50
    -> GROUP BY vend_id
    -> HAVING COUNT(*) >= 2;
+---------+----------+
| vend_id | num_vend |
+---------+----------+
|    1003 |        4 |
+---------+----------+
```

### 视图

```mysql
# 创建视图
mysql> CREATE VIEW t AS
    -> SELECT cust_name, cust_contact, prod_id
    -> FROM customers, orders, orderitems
    -> WHERE customers.cust_id = orders.cust_id
    ->  AND orderitems.order_num = orders.order_num;
Query OK, 0 rows affected (0.02 sec)

# 显示视图里的信息
mysql> SELECT * FROM t;
+----------------+--------------+---------+
| cust_name      | cust_contact | prod_id |
+----------------+--------------+---------+
| Coyote Inc.    | Y Lee        | ANV01   |
| Coyote Inc.    | Y Lee        | ANV02   |
| Coyote Inc.    | Y Lee        | TNT2    |
| Coyote Inc.    | Y Lee        | FB      |
| Coyote Inc.    | Y Lee        | FB      |
| Coyote Inc.    | Y Lee        | OL1     |
| Coyote Inc.    | Y Lee        | SLING   |
| Coyote Inc.    | Y Lee        | ANV03   |
| Wascals        | Jim Jones    | JP2000  |
| Yosemite Place | Y Sam        | TNT2    |
| E Fudd         | E Fudd       | FC      |
+----------------+--------------+---------+

# 从视图中查询 prod_id = 'TNT2' 的顾客信息
mysql> SELECT cust_name, cust_contact FROM t WHERE prod_id = 'TNT2';
+----------------+--------------+
| cust_name      | cust_contact |
+----------------+--------------+
| Coyote Inc.    | Y Lee        |
| Yosemite Place | Y Sam        |
+----------------+--------------+
```

























