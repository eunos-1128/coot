
# every month or so
xlim=80 # was 60 # was 40
ylim=30 # was 20 # was 16 # reasonable scope

prediction_text_x_placement = 30
prediction_text_y_placement = 0

legend_x = 44
legend_y = 3

non_converging_x = 22
non_converging_y = 1

source('arrow.r')

predict = function(x_pos, y_pos) {

                                        # 1 is day number
                                        # 2 is menu item scope
                                        # 3 is done menu item
                                        # 4 is other items scope
                                        # 5 is done other items

    menu_item_scope_start = a$V2[1] + a$V3[1]
    menu_item_scope_end   = a$V2[length(a$V2)] + a$V3[length(a$V2)]

    menu_item_done_start = a$V3[1]
    menu_item_done_end   = a$V3[length(a$V3)]

                                        # s = paste(scope_start, scope_end)
                                        # print(s)
                                        # s = paste(done_start, done_end)
                                        # print(s)

    now_day = a$V1[length(a$V1)]

                                        # print(now_day)

    c1 = menu_item_done_start
    c2 = menu_item_scope_start
    m1 = (menu_item_done_end - menu_item_done_start)/now_day
    m2 = (menu_item_scope_end - menu_item_scope_start)/now_day

    m_diff = m1 - m2

                                        # print(paste("m1" + m1))
                                        # print(paste("m2" + m2))
                                        # print(paste("m_diff" + m_diff))

    # print("m1")
    # print(m1)
    # print("m2")
    # print(m2)
    print("m_diff")
    print(m_diff)

    if (m_diff >= 0) {
      X_pred = (c2-c1)/(m1-m2)
      Y_pred_1 = m1 * X_pred + c1
      Y_pred_2 = m2 * X_pred + c2
      r = paste(X_pred+1, Y_pred_1)
      r = paste(r, now_day)
      # print(r)
      days_delta = X_pred - now_day
      today_t = Sys.Date()
      predict_t = today_t + days_delta
      date_s = format(predict_t, format="%d %B %Y")
      t = paste('Projected Completion Day:\n', date_s)
      text(x_pos, y_pos, t, pos=3, cex=0.8)
      s = 3 # should depend on xlim, 3 is good when xlim is 200
      # the ratio between s_x and s_y is the ratio of
      # xlim and y_lim
      s_x = 0.008*xlim
      s_y = 0.02 *ylim
      rect(X_pred-s_x, 0.5*(Y_pred_1-s_y), X_pred+s_x, 0.5*(Y_pred_1+s_y), col = 'darkgreen')
      # need list of X values, list of Y values
      # not x,y pairs
      # lines(c(X_pred,X_pred), c(0, Y_pred_1), col='darkgreen', lty=2)
      lines(c(0,X_pred), c(0,      Y_pred_1*0.5), col='darkgreen', lty=2)
      lines(c(0,X_pred), c(c2*0.5, Y_pred_1*0.5), col='darkgreen', lty=2)

      return(r)

    } else {

        nct = paste('Non-converging:', round(m_diff, digits=3))
        print(nct)
        legend(non_converging_x, non_converging_y, nct, cex=0.8, box.lwd=0)

        ## (x-values), (y-values)

        lines(c(0, now_day), c(0, menu_item_done_end*0.5),  col='#202020', lty=2)
        lines(c(0, now_day), c(menu_item_scope_start*0.5, menu_item_scope_end*.5), col='#202020', lty=2)

   }
}


a = read.table('burn-up.tab')


# change the plot resolution
png('burn-up.png', res=92, width=640, height=640)
# png('burn-up.png')

do_plot = function() {

    ## the columns are time(days) n-items-todo n-items-done

    plot(ylim=c(0,ylim), xlim=c(0,xlim), NULL, NULL, t='n', cex.main=1.0,
               main="Coot 1.1.16 Development Progress",
               xlab="Real Days since records began (30 Mar 2025)",
               ylab="Scope & Work Done (Days)")

    grid()
                                        # V1 is "days"
                                        # V2 is "what's left"
                                        # V3 is done
                                        # V4 is what's left others
                                        # V5 is what's done others
    points(a$V1, (a$V2+a$V3)*0.5, t='l', lwd=3, lty=1, col='blue')
    points(a$V1, a$V3*0.5, t='l', lwd=3, lty=1, col='forestgreen')

                                        # Total scope
    # points(a$V1, (a$V2+a$V3+a$V4+a$V5)*0.5, lwd=3, t='l', col='blue')
                                        # Total done
    # points(a$V1, (a$V3+a$V5)*0.5, t="l", lwd=3, col="forestgreen")

    leg.txt <- c("Total Completed", "Total Scope")
    legend(legend_x, legend_y, legend=leg.txt, col=c("forestgreen", "blue"), lty=1:1, lwd=3, cex=0.7)

    # leg.txt <- c("Menu Items Scope", "Menu Items Completed", "Total Completed", "Total Scope")
    # legend(legend_x, legend_y, legend=leg.txt, col=c("brown", "black", "forestgreen", "blue"), lty=1:1, lwd=3, cex=0.7)
}

do_plot()

predict(prediction_text_x_placement, prediction_text_y_placement)

betterArrow(60, 21, 52, 18, 0.2, col='grey30', code=2)
text(60, 24., labels="Postpone\nCarbohydrate\nand\nTexture Map", col='grey30', cex=1.0)

dev.off()

