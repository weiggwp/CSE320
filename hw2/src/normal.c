
/*
 * Normalize scores, using the computed statistics.
 */

#include<stddef.h>
#include<stdio.h>
#include <string.h>
#include "global.h"
#include "gradedb.h"
#include "stats.h"
#include "allocate.h"
#include "normal.h"
#include "error.h"


/*
 * Normalize scores:
 *      For each student in the course roster,
 *       for each score for that student,
 *              compute the normalized version of that score
 *              according to the substitution and normalization
 *              options set for that score and for the assignment.
 */

void normalize(c)
Course *c;
{
        Student *student_p;
        Score *rawscores_p, *normscores_p;
        Classstats *classstats_p;
        Sectionstats *sectionstats_p;

        //for each student in the course roster
        for(student_p = c->roster; student_p != NULL; student_p = student_p->cnext) {

          //initialize norm score
           student_p->normscores = normscores_p = NULL;

           //for each score for current student
           for(rawscores_p = student_p->rawscores; rawscores_p != NULL; rawscores_p = rawscores_p->next) {
              //get the class stats
              classstats_p = rawscores_p->cstats;
              //get the section stats
              sectionstats_p = rawscores_p->sstats;

              if(normscores_p == NULL) {
                student_p->normscores = normscores_p = newscore();
                normscores_p->next = NULL;
              } else {
                normscores_p->next = newscore();
                normscores_p = normscores_p->next;
                normscores_p->next = NULL;
              }
              normscores_p->asgt = rawscores_p->asgt;
              normscores_p->flag = rawscores_p->flag;
              normscores_p->subst = rawscores_p->subst;
              if(rawscores_p->flag == VALID) {
                normscores_p->grade = normal(rawscores_p->grade, classstats_p, sectionstats_p);
              } else {
                switch(rawscores_p->subst) {
                case USERAW:
                        normscores_p->grade = normal(rawscores_p->grade, classstats_p, sectionstats_p);
                        break;
                case USENORM:
                        if(rawscores_p->asgt->npolicy == QUANTILE)
                                normscores_p->grade = rawscores_p->qnorm;
                        else
                                normscores_p->grade = rawscores_p->lnorm;
                        break;
                case USELIKEAVG:
                        normscores_p->grade = studentavg(student_p, classstats_p->asgt->atype);
                        break;
                case USECLASSAVG:
                        if(rawscores_p->asgt->npolicy == QUANTILE)
                                normscores_p->grade = 50.0;
                        else
                                normscores_p->grade = rawscores_p->asgt->mean;
                        break;
                }
              }
         }
        }
}

/*
 * Normalize a raw score according to the normalization policy indicated.
 */

float normal(s, classstats_p, sectionstats_p)
double s;
Classstats *classstats_p;
Sectionstats *sectionstats_p;
{
        Assignment *a;
        Freqs *fp;
        int n;

        a = classstats_p->asgt;
        switch(a->npolicy) {
        case RAW:
                return(s);
        case LINEAR:
                switch(a->ngroup) {
                case BYCLASS:
                        if(classstats_p->stddev < EPSILON) {
                           warning("Std. dev. of %s too small for normalization.",
                                 classstats_p->asgt->name);
                           classstats_p->stddev = 2*EPSILON;
                         }
                        return(linear(s, classstats_p->mean, classstats_p->stddev, a->mean, a->stddev));
                case BYSECTION:
                        if(sectionstats_p->stddev < EPSILON) {
                           warning("Std. dev. of %s, section %s too small for normalization.",
                                 sectionstats_p->asgt->name, sectionstats_p->section->name);
                           sectionstats_p->stddev = 2*EPSILON;
                         }
                        return(linear(s, sectionstats_p->mean, sectionstats_p->stddev, a->mean, a->stddev));
                }
        case SCALE:
                if(a->max < EPSILON) {
                  warning("Declared maximum score of %s too small for normalization.",
                        classstats_p->asgt->name);
                  a->max = 2*EPSILON;
                }
                return(scale(s, a->max, a->scale));
        case QUANTILE:
                switch(a->ngroup) {
                case BYCLASS:
                        fp = classstats_p->freqs;
                        n = classstats_p->tallied;
                        if(n == 0) {
                           warning("Too few scores in %s for quantile normalization.",
                                   classstats_p->asgt->name);
                           n = 1;
                         }
                        break;
                case BYSECTION:
                        fp = sectionstats_p->freqs;
                        n = sectionstats_p->tallied;
                        if(n == 0) {
                           warning("Too few scores in %s, section %s for quantile normalization.",
                                 sectionstats_p->asgt->name, sectionstats_p->section->name);
                           n = 1;
                         }
                        break;
                }
                /*
                 * Look for score s in the frequency tables.
                 * If found, return the corresponding percentile score.
                 * If not found, then use the percentile score corresponding
                 * to the greatest valid score in the table that is < s.
                 */

                for( ; fp != NULL; fp = fp->next) {
                   if(s < fp->score)
                        return((float)fp->numless*100.0/n);
                   else if(s == fp->score)
                        return((float)fp->numless*100.0/n);
                }
        }
  return s;
}

/*
 * Perform a linear transformation to convert score s,
 * with sample mean rm and sample standard deviation rd,
 * to a normalized score with normalized mean nm and
 * normalized standard deviation nd.
 *
 * It is assumed that rd is not too small.
 */

float linear(s, rm, rd, nm, nd)
double s, rm, rd, nm, nd;
{
        return(nd*(s-rm)/rd + nm);
}

/*
 * Scale normalization rescales the score to a given range [0, scale]
 *
 * It is assumed that the declared max is not too small.
 */

float scale(s, max, scale)
double s, max, scale;
{
        return(s*scale/max);
}

/*
 * Compute a student's average score on all assignments of a given type.
 * If a weighting policy is set, we use the specified relative weights,
 * otherwise all the assignments of the type are weighted equally.
 */

float studentavg(s, t)
Student *s;
Atype t;
{
        int n, wp;
        double sum;
        Score *scp;
        float f, w;

        n = 0;
        wp = 0;
        sum = 0.0;
        w = 0.0;
        for(scp = s->rawscores; scp != NULL; scp = scp->next) {
           if(!strcmp(scp->asgt->atype, t) &&
              (scp->flag == VALID || scp->subst == USERAW)) {
                n++;
                f = normal(scp->grade, scp->cstats, scp->sstats);
                if(scp->asgt->wpolicy == WEIGHT) {
                   wp = 1;
                   sum += f * scp->asgt->weight;
                   w += scp->asgt->weight;
                } else {
                   sum += f;
                }
           }
        }
        if(n == 0 || w == 0.0) {
                warning("Student %s %s has no like scores to average,\n%s",
                        s->name, s->surname, "using raw 0.0.");
                return(0.0);
        } else {
                if(wp) return(sum/w);
                else return(sum/n);
        }
}

/*
 *  Compute composite scores:
 *      For each student in the course roster,
 *        For each assignment,
 *         Find the student's normalized score for that assignment,
 *              and include it in the weighted sum.
 */

void composites(c)
Course *c;
{
        Student *student_p;
        Score *scp;
        Assignment *ap;
        float sum;
        int found;

        for(student_p = c->roster; student_p != NULL; student_p = student_p->cnext) {
           sum = 0.0;
           for(ap = c->assignments; ap != NULL; ap = ap->next) {
              found = 0;
              for(scp = student_p->normscores; scp != NULL; scp = scp->next) {
                if(scp->asgt == ap) {
                   found++;
                   sum += scp->grade * (ap->wpolicy == WEIGHT? ap->weight: 1.0);
                }
              }
              if(!found) {
                warning("Student %s %s has no score for assignment %s.",
                        student_p->name, student_p->surname, ap->name);
              }
           }
           student_p->composite = sum;
        }
}
