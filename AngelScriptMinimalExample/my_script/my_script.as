
float as_main(FlagAndVector3 value)
{
   const float result = value.manhattan();
   println("AS: value.manhattan(): " + result);

   if (value.flag) {
        println("AS: value.flag: true");
    } else {
        println("AS: value.flag: false");
    }
    
    const bool f = value  // <-- FlagAndVector3 の operator bool() が呼ばれる 
        && value.flag;
    println("AS: value && value.flag: " + f);
    
    return value.manhattan();
}
