**Project to learn and practice multithreading.**

This program takes in a text file as input and uses multiple threads to process and compress it as if it were being zipped. Here is example input and output:
- Input file content: aaeeoooooeee
- Output in plain text: a 2 e 2 o 2 o 3 e 3
The input size and number of threads used can vary.

The important code to be reviewed is in src/pzip.c. In it you will find the function for handling zipping in an array of characters in parallel and a function to handle thread callbacks.

If you plan to run this, I recommend running it in the provided Docker container, else it may not compile properly on your system. First run make to compile pzip on your system. The usage of pzip is as follows:
- ./pzip INPUT_FILE OUTPUT_FILE N_THREADS

Test input files are provided in the test folder, along with their expected outputs. You may also generate characters by using generate_chars.py as such:
- "./generate_chars.py NUM > test_input", where NUM is the number of characters. You may also name test_input whatever you like, of course.
