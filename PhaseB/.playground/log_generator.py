import sys
import os
import re

def get_input_filename():
        if len(sys.argv) < 2:
                current_script_filename = os.path.basename(__file__)
                print(f'Script {current_script_filename} expected .y filename as start-up argument.')
                sys.exit(1)
        return sys.argv[1]

def extract_rule_header_from_pattern(matched_rule_pattern: str):
        return matched_rule_pattern.replace(':','').strip()
        
def replace_hooks(input_file_path: str, output_file_path: str, hook: str, function_name):
        # We escape hook string, as it might contain escape characters.
        hook_definition_pattern = re.compile(rf'^\s*#define\s+{re.escape(hook)}\b') 
        rule_header_pattern = re.compile(r'^(\s*)([a-zA-Z_][a-zA-Z0-9_]*)(\s*):')
        rule_suffix_pattern = re.compile(r'[a-zA-Z_][a-zA-Z0-9_]*')
        injected_code_pattern = re.compile(r'\{.*?\}')

        with open(input_file_path, 'r') as infile:
                lines = infile.readlines()

        parsed_code_lines = []
        before_grammar_rules = True
        rule_header_obtained = False
        parsing_rule_suffix = False
        current_rule_header = ''
        suffix_tokens = []
        for line in lines:
                if hook_definition_pattern.match(line): # Parsed codes does contain the hook definition.
                        continue
                if before_grammar_rules:
                        parsed_code_lines.append(line)
                        if '%%' in line:
                                before_grammar_rules = False
                        continue
                if not rule_header_obtained and rule_header_pattern.match(line):
                        current_rule_header = extract_rule_header_from_pattern(line)
                        rule_header_obtained = True
                        parsing_rule_suffix = True
                        parsed_code_lines.append(line)
                        continue
                if (parsing_rule_suffix):
                        if ';' in line:
                                parsed_code_lines.append(line)
                                parsing_rule_suffix = False
                                rule_header_obtained = False
                                current_rule_header = ''
                                continue
                        line_cleaned_injected_code = injected_code_pattern.sub('', line) # Remove injected code from line.
                        suffix_tokens_with_hook = rule_suffix_pattern.findall(line_cleaned_injected_code)
                        suffix_tokens.extend([token for token in suffix_tokens_with_hook if token != hook])
                        if hook not in line:
                                parsed_code_lines.append(line)
                                continue
                        #If control reaches here, we have a hook.
                        suffix_str = ' '.join(suffix_tokens)
                        hook_replacer = f'{function_name}("{current_rule_header}", "{suffix_str}")'
                        line_with_injected_hook = line.replace(hook, hook_replacer)
                        parsed_code_lines.append(line_with_injected_hook)
                        suffix_tokens.clear()
                        continue

        with open(output_file_path, 'w') as outfile:
                outfile.writelines(parsed_code_lines)
 
        


def main():
        bison_filename = get_input_filename()
        replace_hooks(bison_filename, 'OUTPUT.y', '__LOG__', 'displayLOGG')



if __name__ == "__main__":
        main()

    