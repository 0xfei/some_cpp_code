/**
 * test scala analyzer
 */
import java.util.Map

class Define {
  def x(value: => Any): String = "x"
  def y(f: Int => String): String = "y"
  def z(f: (Int, Int) => Int): String = "z"
  def m(f: Map[String, String] => String): String = "m"
  def r(value: String*): String = "r"
}

class PartialFunction {
  List(1,2,3,4).collect(
    x => x match { case _ if x < 3 => x }
  )

  List(1,2,3,4).collect {
    case x: Int => x
  }
}

object Test {
  def main(args: Array[String]) {
    /** println Hello World
     */
    println("Hello world")
  }
}

