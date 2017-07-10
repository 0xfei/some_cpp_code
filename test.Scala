/**
 * test scala analyzer
 */
object Test {
  def main(args: Array[String]) {
    List(1,2,3,4).collect(
      x => x match { case _ if x < 3 => x }
    )

    List(1,2,3,4).collect {
      case x: Int => x
    }
  }
}

