main()
{
	int a;

	printf("overlay tester\n");
	a = ovloader("ov1", "first message");
	printf("in main. ovly1 returned %d\n", a);
	a = ovloader("ov2", "second message");
	printf("in main. ovly2 returned %d\n", a);
}

