def pre_mutation(context):
    # Skip test files
    if 'test_' in context.filename or context.filename.endswith('_test.py'):
        context.skip = True

def pre_mutation_ast(context):
    # Additional filtering if needed
    pass

