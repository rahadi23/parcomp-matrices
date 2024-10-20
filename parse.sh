# take 1 argument: directory containing the output files
dir=${1:-.}
files=$(ls $dir)

# printing the csv header
echo "alg;env;n;np;p;key;val"

for file in $files; do 
  # extract the algorithm, environment, n, np from the file name
  if [[ "$file" =~ ^(.+)\-(dev|single|single\-node|multiple\-nodes)\-n0*(.*)\-np0*(.*)\.(out|log)$ ]]; then
    alg=${BASH_REMATCH[1]}
    env=${BASH_REMATCH[2]}
    n=${BASH_REMATCH[3]}
    np=${BASH_REMATCH[4]}

    while IFS= read -r line; do
      # extract the key and value from file content lines
      if [[ "$line" =~ ^\ *([0-9]*)\:?\ *\[INFO\]\ *([a-zA-Z\.\ ]*)\ *\:\ *(.*)$ ]]; then
        p=${BASH_REMATCH[1]}
        key=${BASH_REMATCH[2]}
        val=${BASH_REMATCH[3]}
        echo "${alg};${env};${n};${np};${p};${key};${val}"
      fi
    done < "$dir/$file"
  fi
done
