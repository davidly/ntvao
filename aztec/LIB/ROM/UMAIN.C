/*
 * _main is an interface between the Manx-supplied, 
 * assembly language startup routine .begin, 
 * and the user-written, C language routine main().
 * _main can optionally do system-type activities such as redirecting
 * I/O streams, and parsing command line arguments that are then
 * passed to main() as its argc and argv parameters.
 */

_main()
{
	main();
}
