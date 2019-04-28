# this is an example file with some code you might find in an
# ucuetis program. note that all of this is tentative, and subject
# to change.

# print the string literal "hello" to stdout.
# the pipe (<>) operator redirects input from
# one thing to another. OUT is a special keyword
# that simply means "take whatever you get and
# send it to stdout".
(<> OUT "hello")

# store a value in a variable.
(defn x 100)

# you can use a variable in computations:
(+ x 20)

# if you want to add 20 to x and store the result
# back in x:
(defn x (+ x 20))

# defnine a list of integers...
(defn numbers { 1, 2, 3, 4, 5 })

# ... and iterate over it. the 'yield' keyword takes 
# the current value from within the loop and "returns" 
# it to the outer scope. in this case, it becomes the 
# input for a pipe to stdout. 'it' is a keyword that 
# refers to the current iteration target.
(<> OUT (for (numbers) (yield it)))

# defnine addFn ...
(defn addFn fn(a: num, b: num) => num ((+ a b)))

# ... and call it:
(<> OUT addFn[2 2])

# the final expression of a function is it's return value.
# in this case, the value of x will be returned, and then
# x will be discarded as expected.
(defn pointlessAssign fn(a: num, b: num) => num ( (defn x (+ a b)) ))

# you can use the yield keyword to explicity return a value as well:
(defn explicitReturn fn(a: num, b: num) => num ( (yield (+ a b)) ))

# if conditionals:
(if (< x 20) (+ x 300) (+ x 200))

# while loops:
(while (> x 200000) (* x 2))

# WARNING! function contracts have zero support currently! trying to use them
# will result in syntax errors, none of which will directly tell you about
# contracts not being supported. the syntax here is also ancient and not
# even close to the current version.

# functions will support precondition 'contracts' like so:
#<< positiveAddFn fn<(< a 0) | (< b 0)>(a: num, b: num) => num [
#    + a b;
#];

# the above contract requires that any input to positveAddFn be
# greater than zero. this will fail at compile time:
#positiveAddFn(-2 -2);

# if a contract cannot be enforced at compile time, it will be
# enforced at runtime. the following uses another special keyword
# (akin to OUT) to read in some user input, then attempt to sum
# them with positiveAddFn:

#<< first IN;
#<< second IN;

#positiveAddFn(first second);

# if the values fail to meet the contract requirements, the program will
# error.


# more soon...