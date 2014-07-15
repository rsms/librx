# FuncDef  = "func" Whitespace? "(" Whitespace? FuncArgs? Whitespace? ")" (Whitespace? Type)?
# FuncArgs = FuncArg | FuncArg Whitespace? "," Whitespace? FuncArgs
# FuncArg  = Name Whitespace Type | Name | Type
#
# Legend:
#   Symbol: foo
#   Comment: #'foo'
#   Text: "foo"
#   List: [ ... ]
#   Block: { ... }
#   Group: ( ... )
#
# Parsed representation: (where whitespace is ignored)
#
# [ package, example, {
#   [ #'hh:' ],
#   [ #' namespace example {' ],
#   [ interface, Person, {
#     [ firstname, Text ],
#     [ lastname, Text ],
#   }],
#   [ #'Purely compile-time' ],
#   [ func, greeter, ( [person, Person], [prefix, Text] ), {
#     [ "Hello, #{person.firstname} #{person.lastname}" ]
#   }]
# }]
#

package example
  #hh:
  # namespace example {

  interface Person
    firstname Text
    lastname Text
  #Purely compile-time

  func greeter(person Person, prefix Text)
    "Hello, #{prefix} #{person.firstname} #{person.lastname}"
  #hh:
  # Text greeter(Person);
  #
  #cc:
  # Text greeter(Person person) {
  #   return Text{"Hello, "} + person.firstname() + " " + person.lastname();
  # }

  interface Geometry
    area Number

  main = func
    Shape = struct
      color = 0xff0000
      area = 0

    Square = Shape
      width = 100
      area = @width * @width

    Rectangle = Square
      height = 50
      area = @width * @height

    mySquare = Square{ width = 10 }

    func <self Geometry>fillColor(color Number)
      color / self.area

    console.log Square.area             # 10000
    console.log mySquare.area           # 100
    console.log Square{width = 15}.area  # 225
    console.log Rectangle.area          # 5000
    console.log Rectangle:fillColor     # 3342

package example

  type Student
    fullname Text
    constructor(firstname, initial, lastname Text)
      @fullname = firstname + " " + initial + " " + lastname
  #hh:
  # struct Student { RX_REF_MIXIN(Student)
  #   Text& fullname() const { CHECK_NULL(self); return self->_fullname; }
  #   Student(Text,Text,Text);
  # };
  #
  #cc:
  # struct Student::Imp : rx::ref_counted {
  #   Text _fullname;
  #   Imp(Text fullname) : _fullname{fullname} {}
  # };
  # Student::Student(Text firstname, Text initial, Text lastname)
  #   : self{new Imp{firstname + " " + initial + " " + lastname}}
  #   {}
  #

  user = Student{firstname: "Jane", lastname: "User", initial: "Mr"}
  #cc:
  # Student user{"Jane", "Mr", "User"};

  console.log(greeter(user))
  #cc:
  # console.log(greeter(user))

#
