informe: 
	@echo "Compilando"
	@cd src; \
	pdflatex --interaction batchmode informe.tex > /dev/null; \
	pdflatex --interaction batchmode informe.tex > /dev/null; \
	rm -f *.toc *.out *.aux *.log
	@mv src/informe.pdf .
	@echo "Listo"
	                      
clean:
	rm -f informe.pdf 
